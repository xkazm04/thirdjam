// Lane A56: the lockstep soak. Hundreds of joined matches in one process, two peers through the real UJamLockstepDriver and the real packet
// bytes, faults injected by the link between them. Names are Bar.Net.* so the gate's "Jam" filter never runs them (they take minutes).
//
//   Bar.Net.SoakClean     N seeds x hockey.regular at pro, a regular playtester (host) against a good one (guest) as the two humans, 3600 frames,
//                         a LAN-like link (1 tick each way). The two peers' full states are compared every frame (bit for bit, every reflected field
//                         of every skater and the puck, the rule / zone / effect hashes); the driver's own 60-frame checkpoint runs as in the game.
//   Bar.Net.SoakLossy     the same over fault profiles: loss 2 / 10 / 30 %, delay 0..6 ticks (jitter, so packets reorder), duplicates, a blackout.
//   Bar.Net.SoakMismatch  a protocol-4 peer, unknown flag bits, packets of the wrong size, corrupted bytes, against a protocol-5 host.
//
// Run (headless, its own editor process; tools/soak/lockstep/run.ps1 wraps it and splits seeds over processes):
//   UnrealEditor-Cmd.exe Game/ThirdJam.uproject -ExecCmds="Automation RunTests Bar.Net.SoakClean; Quit" -nullrhi -unattended -nosplash -log
//     -SoakSeedFirst=1 -SoakSeeds=200 -SoakFrames=3600 -SoakProfiles=loss2,loss10 -SoakOut=<file.jsonl>
// Every match writes one SOAKLOCK JSON line (log and -SoakOut); a desync also writes both states at the first differing frame.
// What the bot-side inputs are: ALobbyGameMode::GatherBotInputs (zones, pickups, bDeke, bShootHeld false), not JamNetTests' older copy.
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformTime.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "UObject/UnrealType.h"
#include "UObject/StrongObjectPtr.h"
#include "JamCartridge.h"
#include "JamSim.h"
#include "JamBrain.h"
#include "JamSimAdapter.h"
#include "JamPlaytester.h"
#include "JamEffectSystem.h"
#include "JamRuleSystem.h"
#include "JamZones.h"
#include "JamEventBus.h"
#include "JamRandom.h"
#include "JamNetPackets.h"
#include "JamLink.h"
#include "JamLockstepDriver.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamNetSoak
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

	// ---- the faulty wire ----

	struct FFaults
	{
		FString Name = TEXT("clean");
		double Loss = 0.0;          // probability a datagram is dropped (seeded)
		int32 DelayMin = 1;         // ticks from send to receivable, uniform in [DelayMin, DelayMax]; a spread reorders
		int32 DelayMax = 1;
		double Dup = 0.0;           // probability a delivered datagram arrives twice (its own delay)
		int64 BlackoutAt = -1;      // from this tick, every datagram both ways is lost for BlackoutTicks
		int32 BlackoutTicks = 0;
		double Corrupt = 0.0;       // B's datagrams: probability one random byte past the header is flipped (UDP checksum off)
		uint16 OrFlagsFromB = 0;    // B's input packets carry these flag bits on every input word (an unknown bit)
		uint8 VersionFromB = 0;     // B's datagrams carry this version byte (0 = the real one)
		bool bUdp = false;          // the real FJamUdpLink pair on 127.0.0.1 instead of the ticked link (ticks paced at UdpTickUs of wall clock)
		int32 UdpTickUs = 250;
		int32 BEvery = 1;           // peer B steps only every BEvery ticks (a slower machine: 2 = B at 30 Hz)
		bool bExpectLost = false;   // the profile is meant to end in "connection lost"
		int64 InjectFrame = -1;     // lane A120: see FPeer::InjectFrame (the guest's machine, the guest's seat)
		bool bExpectDesync = false;
	};

	class FSoakLink
	{
	public:
		struct FPacket { int64 Ready = 0; int64 Seq = 0; TArray<uint8> Bytes; };
		struct FEnd : public IJamLink
		{
			FSoakLink* Owner = nullptr;
			TArray<FPacket>* Outbox = nullptr;
			TArray<FPacket>* Inbox = nullptr;
			bool bIsB = false;
			int64 NextSeq = 0, LastDelivered = -1;
			int32 Run = 0;
			// what the wire did to this end's outgoing datagrams
			int32 Dropped = 0, Duplicated = 0, Blacked = 0, MaxRun = 0, Corrupted = 0;
			// what this end received out of order (a lower sequence than one already delivered) or twice
			int32 Reordered = 0, Twice = 0;

			bool Send(const TArray<uint8>& In) override
			{
				BytesSent += In.Num();
				++PacketsSent;
				const int64 Seq = NextSeq++;
				const FFaults& Fz = Owner->F;
				const bool bBlack = Fz.BlackoutAt >= 0 && Owner->Tick >= Fz.BlackoutAt && Owner->Tick < Fz.BlackoutAt + Fz.BlackoutTicks;
				const bool bLose = Fz.Loss > 0.0 && Owner->Rng.NextUnit() < Fz.Loss;
				if (bBlack || bLose)
				{
					if (bBlack) ++Blacked; else ++Dropped;
					MaxRun = FMath::Max(MaxRun, ++Run);
					return true;
				}
				Run = 0;
				TArray<uint8> Bytes = In;
				if (bIsB) Owner->MutateFromB(Bytes, *this);
				auto Delay = [&]() { return Fz.DelayMax > Fz.DelayMin ? Fz.DelayMin + Owner->Rng.NextInt(Fz.DelayMax - Fz.DelayMin + 1) : Fz.DelayMin; };
				Outbox->Add({ Owner->Tick + Delay(), Seq, Bytes });
				if (Fz.Dup > 0.0 && Owner->Rng.NextUnit() < Fz.Dup) { Outbox->Add({ Owner->Tick + Delay(), Seq, Bytes }); ++Duplicated; }
				return true;
			}

			bool Recv(TArray<uint8>& Out) override
			{
				int32 Best = INDEX_NONE;
				for (int32 i = 0; i < Inbox->Num(); ++i)
				{
					const FPacket& P = (*Inbox)[i];
					if (P.Ready > Owner->Tick) continue;
					if (Best == INDEX_NONE || P.Ready < (*Inbox)[Best].Ready || (P.Ready == (*Inbox)[Best].Ready && P.Seq < (*Inbox)[Best].Seq)) Best = i;
				}
				if (Best == INDEX_NONE) return false;
				const int64 Seq = (*Inbox)[Best].Seq;
				if (Seq < LastDelivered) ++Reordered;
				else if (Seq == LastDelivered) ++Twice;
				LastDelivered = FMath::Max(LastDelivered, Seq);
				Out = MoveTemp((*Inbox)[Best].Bytes);
				Inbox->RemoveAt(Best);
				BytesRecv += Out.Num();
				++PacketsRecv;
				return true;
			}
			bool IsOpen() const override { return true; }
		};

		FSoakLink(const FFaults& InF, uint64 Seed) : F(InF), Rng(Seed)
		{
			A.Owner = this; A.Outbox = &ToB; A.Inbox = &ToA;
			B.Owner = this; B.Outbox = &ToA; B.Inbox = &ToB; B.bIsB = true;
		}
		void Advance() { ++Tick; }

		// The peer-B faults of the mismatch profiles, applied to bytes that survived the wire.
		void MutateFromB(TArray<uint8>& Bytes, FEnd& End)
		{
			if (F.VersionFromB && Bytes.Num() > 4) Bytes[4] = F.VersionFromB;
			if (F.OrFlagsFromB && Bytes.Num() >= FJamNetInputPacket::HeaderBytes && Bytes[5] == static_cast<uint8>(JamNet::EType::Input))
			{
				const int32 Count = Bytes[FJamNetInputPacket::HeaderBytes - 1];
				for (int32 i = 0; i < Count; ++i)
				{
					const int32 Off = FJamNetInputPacket::HeaderBytes + i * FJamNetQuantInput::WireBytes + 8;   // X Y SX SY, then the flags word (host order)
					if (Off + 1 >= Bytes.Num()) break;
					uint16 Fl = 0; FMemory::Memcpy(&Fl, &Bytes[Off], 2); Fl |= F.OrFlagsFromB; FMemory::Memcpy(&Bytes[Off], &Fl, 2);
				}
			}
			if (F.Corrupt > 0.0 && Bytes.Num() > 6 && Rng.NextUnit() < F.Corrupt)
			{
				const int32 At = 6 + Rng.NextInt(Bytes.Num() - 6);
				Bytes[At] ^= static_cast<uint8>(1u << Rng.NextInt(8));
				++End.Corrupted;
			}
		}

		FFaults F;
		FJamRandom Rng;
		FEnd A, B;
		int64 Tick = 0;
	private:
		TArray<FPacket> ToA, ToB;
	};

	// ---- one machine ----

	struct FSnap
	{
		uint64 Lock = 0, SimH = 0, FxH = 0, RulesH = 0, ZonesH = 0, SimLockH = 0;
		TArray<FJamSkaterState> Skaters;
		FJamPuckState Puck;
		int32 Phase = 0, Score0 = 0, Score1 = 0, Timer = 0;
		uint64 Rng = 0;
	};

	struct FTiming
	{
		TArray<uint32> Pump, Try, Sim;   // cycles per call
	};

	FJamInput ToInput(const JamAI::FJamAIInput& A)
	{
		// FJamSimAdapter::Step's copy, field by field (the path the playtester's output takes in every headless runner).
		FJamInput I;
		I.Move = FVector2D(A.Move.X, A.Move.Y);
		I.bShoot = A.bShoot; I.bShootHeld = A.bShootHeld; I.bCheck = A.bCheck; I.bBrake = A.bBrake; I.bPass = A.bPass;
		I.Stick = FVector2D(A.Stick.X, A.Stick.Y);
		I.bPoke = A.bPoke; I.bLift = A.bLift; I.bDeke = A.bDeke; I.bSpin = A.bSpin;
		I.bPassHeld = A.bPassHeld; I.bSprint = A.bSprint; I.bAbility = A.bAbility;   // lane A104: the saucer hold, the sprint and F reach the wire too
		return I;
	}

	struct FPeer
	{
		TStrongObjectPtr<UJamSim> Sim;
		TStrongObjectPtr<UJamRuleSystem> Rules;
		TStrongObjectPtr<UJamZoneSystem> Zones;
		TStrongObjectPtr<UJamEffectSystem> Fx;
		TStrongObjectPtr<UJamBrain> Brain;
		TStrongObjectPtr<UJamLockstepDriver> Driver;
		JamAI::FJamPlaytester Tester;
		int32 LocalSlot = 0, RemoteSlot = 0;
		TArray<FJamInput> Inputs;
		int32 Stalls = 0;
		TMap<int64, FSnap> Snaps;
		// Edge presses: what the local player pressed (merged over a stall, as the driver promises) against what the sim was handed for that frame.
		// Lane A140: presses are counted as read (one per tick a verb's edge is in the local input) and as applied (in the seat's lockstep input); at
		// the end lost = read - applied - still in flight. Before, the expectation OR'd a stall's presses into one, so a fold never counted as lost.
		int32 EdgePressed[16] = {};
		int32 EdgeApplied[16] = {};
		int32 EdgeLost[16] = {};
		int64 FirstEdgeLostFrame = -1;
		int32 Spins = 0;   // spin presses that reached the sim (either human)
		// Lane A104: what the two humans' new verbs did on the wire: frames with a sprint held, frames with a pass held, ability presses applied (either human).
		int32 Verbs[3] = {};
		bool bSprintOn = true;   // -SoakSprint=0 gives back A56's playtesters (no Shift)
		bool bSeatGuest = true;  // lane A110: ALobbyGameMode seats the guest as a human (UJamSim::SeatGuest); -SoakSeatGuest=0 gives back A104's bot seat
		// Lane A120: a seeded one-field divergence (the field-compare proof): after stepping InjectFrame, this peer's skater InjectSkater gets
		// MoveHist[InjectElement].X += 0.25. -1 = never (every real profile).
		int64 InjectFrame = -1;
		int32 InjectSkater = -1, InjectElement = 7;

		void Init(FAutomationTestBase& T, const UJamCartridge& C, uint64 Seed, IJamLink& Link, int32 InLocal, int32 InRemote, JamAI::EPlaytester Kind)
		{
			// As AHokejGameMode::BuildMatch orders it and ALobbyGameMode reuses it: rule hook and zone source before Init, effects after.
			const FJamGameConfig& G = C.Game;
			Sim.Reset(NewObject<UJamSim>());
			Rules.Reset(NewObject<UJamRuleSystem>());
			Rules->Init(G);
			Sim->SetRuleHook(Rules.Get());
			Zones.Reset(NewObject<UJamZoneSystem>());
			Zones->Init(G);
			Sim->SetZoneSource(Zones.Get());
			Sim->Init(G, Seed);
			// Lane A110: as ALobbyGameMode::StartPlay, both machines seat the guest (the seat that is not the cartridge's human) before the first step.
			if (bSeatGuest) Sim->SeatGuest(InLocal == Sim->GetHumanIndex() ? InRemote : InLocal);
			Fx.Reset(NewObject<UJamEffectSystem>());
			TArray<FString> Errors;
			Fx->Init(Sim.Get(), C.RawJson, Errors);
			for (const FString& E : Errors) T.AddError(E);
			Brain.Reset(NewObject<UJamBrain>());
			Brain->ApplyCartridge(C, TEXT("pro"));
			Brain->ResetBrain();
			Driver.Reset(NewObject<UJamLockstepDriver>());
			Driver->Start(&Link, JamNet::InputDelay);
			Tester.Reset(Kind, Seed);
			Tester.ChargeFramesMax = G.Shot.ChargeFramesMax;
			Tester.bSprint = bSprintOn;   // lane A104: the playtesters press Shift (A72's SprintPct, A95's carry sprint)
			LocalSlot = InLocal; RemoteSlot = InRemote;
			Inputs.SetNum(Sim->GetSkaters().Num());
		}

		uint64 LockHash() const { return JamNetLockstepHash(Sim.Get(), Fx.Get(), Rules.Get(), Zones.Get()); }   // ALobbyGameMode::LockstepHash (lane A104: one function)

		// Lane A104: FJamPlaytester's F, as RunPlaytestMatch decides it (JamPlaytester.cpp, lane A84): the shotgun on a shot or into a crowd, the hip check on
		// a check or an opposing carrier within 1.2 m, the slow puck while carrying, the shield while carrying with an opponent within 4 m. No draws.
		static bool PlaytesterAbility(const UJamSim& Sm, int32 P, const FJamInput& I) { return JamNetPlaytesterAbility(Sm, P, I); }   // lane A129: one function, the lobby's scripted seat presses it too

		// ALobbyGameMode::GatherBotInputs' world: the adapter's view plus pickups and slow ice / no-shoot zones.
		void BuildWorld(JamAI::FJamAIWorld& W) const
		{
			JamAI::FJamSimAdapter::BuildWorld(*Sim, W);
			W.NumPickups = 0;
			for (const FJamPickupOnIce& P : Fx->GetPickups())
			{
				if (W.NumPickups >= JamAI::FJamAIWorld::kMaxPickups) break;
				W.PickupKind[W.NumPickups] = P.PickupIndex;
				W.PickupPos[W.NumPickups] = JamAI::FVec2(P.Pos.X, P.Pos.Y);
				++W.NumPickups;
			}
			for (const FJamZoneConfig& Zc : Zones->GetZones())
			{
				if (Zc.Kind != FName(TEXT("slow_ice")) || W.NumSlowIce >= JamAI::FJamAIWorld::kMaxSlowIce) continue;
				const bool bRect = Zc.Shape == FName(TEXT("rect"));
				W.SlowIceCentre[W.NumSlowIce] = JamAI::FVec2(Zc.X, Zc.Z);
				W.SlowIceRadius[W.NumSlowIce] = bRect ? 0.5 * FMath::Sqrt(Zc.W * Zc.W + Zc.D * Zc.D) : Zc.R;
				++W.NumSlowIce;
			}
			for (int32 i = 0; i < W.NumSkaters; ++i)
			{
				FName ZoneId;
				W.Skaters[i].bNoShoot = Zones->BlocksShot(FVector2D(W.Skaters[i].Pos.X, W.Skaters[i].Pos.Y), ZoneId);
			}
		}

		void Pump(FTiming& Tm)
		{
			const uint64 C0 = FPlatformTime::Cycles64();
			Driver->Pump();
			Tm.Pump.Add(static_cast<uint32>(FPlatformTime::Cycles64() - C0));
		}

		// One attempt, as ALobbyGameMode::StepLockstep: the local human's controls read first, the driver gates, the bots think on the stepped state.
		bool Step(FTiming& Tm)
		{
			JamAI::FJamAIWorld W;
			BuildWorld(W);
			JamAI::FJamAIWorld Mine = W;
			Mine.PlayerIndex = LocalSlot;   // the playtester plays this machine's seat (the guest's is a bot slot in the sim)
			FJamInput Local = ToInput(Tester.Think(Mine));
			Local.bAbility = PlaytesterAbility(*Sim, LocalSlot, Local);   // lane A104: F, as RunPlaytestMatch presses it
			const uint16 Edges = FJamNetQuantInput::From(Local).Flags & FJamNetQuantInput::EdgeFlags;
			const int64 F0 = Driver->GetFrame();
			FJamInput L, R;
			const uint64 C0 = FPlatformTime::Cycles64();
			const bool bOk = Driver->TryStep(Local, L, R);
			Tm.Try.Add(static_cast<uint32>(FPlatformTime::Cycles64() - C0));
			if (bOk || F0 > 0) for (int32 b = 0; b < 16; ++b) if (Edges & (1 << b)) ++EdgePressed[b];   // the driver reads nothing before the first frame
			if (!bOk) { ++Stalls; return false; }
			const uint16 Got = FJamNetQuantInput::From(L).Flags & FJamNetQuantInput::EdgeFlags;
			for (int32 b = 0; b < 16; ++b) if (Got & (1 << b)) ++EdgeApplied[b];
			if (L.bSpin || R.bSpin) ++Spins;
			Verbs[0] += (L.bSprint ? 1 : 0) + (R.bSprint ? 1 : 0);
			Verbs[1] += (L.bPassHeld ? 1 : 0) + (R.bPassHeld ? 1 : 0);
			Verbs[2] += (L.bAbility ? 1 : 0) + (R.bAbility ? 1 : 0);

			const uint64 C1 = FPlatformTime::Cycles64();
			JamAI::FJamAIInput Out[JamAI::kMaxSkaters];
			Brain->Think(W, Out);
			for (int32 i = 0; i < Inputs.Num() && i < JamAI::kMaxSkaters; ++i)
			{
				if (i == W.PlayerIndex) continue;
				FJamInput& I = Inputs[i];
				I.Move = FVector2D(Out[i].Move.X, Out[i].Move.Y);
				I.bShoot = Out[i].bShoot; I.bCheck = Out[i].bCheck; I.bBrake = Out[i].bBrake; I.bPass = Out[i].bPass;
				I.Stick = FVector2D(Out[i].Stick.X, Out[i].Stick.Y);
				I.bPoke = Out[i].bPoke; I.bLift = Out[i].bLift; I.bDeke = Out[i].bDeke;
				// Lane A104: ALobbyGameMode::GatherBotInputs now copies every field the brain sends (FJamSimAdapter::Step's copy).
				I.bShootHeld = Out[i].bShootHeld; I.bPassHeld = Out[i].bPassHeld; I.bSprint = Out[i].bSprint; I.bSpin = Out[i].bSpin; I.bAbility = Out[i].bAbility;
			}
			Inputs[LocalSlot] = L;
			Inputs[RemoteSlot] = R;
			Rules->SetScore(0, Sim->GetScore(0)); Rules->SetScore(1, Sim->GetScore(1));
			Sim->Step(Inputs);
			Zones->Update(Sim->GetFrame(), Sim->GetSkaters(), Sim->GetPuck());
			Fx->Step();
			if (Sim->GetFrame() == InjectFrame && Sim->GetSkaters().IsValidIndex(InjectSkater))
				const_cast<FJamSkaterState&>(Sim->GetSkaters()[InjectSkater]).MoveHist[InjectElement].X += 0.25;   // test only: the sim's own array, one element
			Tm.Sim.Add(static_cast<uint32>(FPlatformTime::Cycles64() - C1));
			Driver->RecordHash(Sim->GetFrame(), LockHash());

			FSnap& S = Snaps.Add(Sim->GetFrame());
			S.Lock = LockHash(); S.SimH = Sim->StateHash(); S.SimLockH = Sim->LockstepStateHash(); S.FxH = Fx->StateHash(); S.RulesH = Rules->StateHash(); S.ZonesH = Zones->StateHash();
			S.Skaters = Sim->GetSkaters();
			S.Puck = Sim->GetPuck();
			S.Phase = static_cast<int32>(Sim->GetPhase()); S.Score0 = Sim->GetScore(0); S.Score1 = Sim->GetScore(1); S.Timer = Sim->GetTimerFrames();
			S.Rng = Sim->GetRandom().GetState();
			return true;
		}
	};

	int32 GuestSlot(const UJamSim& Sim)
	{
		// UJamLobbySubsystem::ComputeCartridgeHash's rule: team 1's first skater that is not the back.
		for (const FJamSkaterState& S : Sim.GetSkaters()) if (S.Team == 1 && S.Role != FName(TEXT("back"))) return S.Index;
		for (const FJamSkaterState& S : Sim.GetSkaters()) if (S.Team == 1) return S.Index;
		return -1;
	}

	FString PropText(const FProperty* P, const void* Container, int32 ArrayIndex = 0)
	{
		FString Out;
		P->ExportTextItem_Direct(Out, P->ContainerPtrToValuePtr<void>(Container, ArrayIndex), nullptr, nullptr, PPF_None);
		return Out;
	}

	// Lane A120 (track Q1): one reflected field compared whole. Identical_InContainer's ArrayIndex defaults to 0, so a C array (MoveHist[10]) was compared on
	// element 0 only; every element now, named with its index. ("" = equal.)
	FString FieldDiff(const FProperty* P, const void* X, const void* Y, const FString& Prefix)
	{
		for (int32 k = 0; k < P->ArrayDim; ++k)
		{
			if (P->Identical_InContainer(X, Y, k)) continue;
			const FString Name = P->ArrayDim > 1 ? FString::Printf(TEXT("%s%s[%d]"), *Prefix, *P->GetName(), k) : Prefix + P->GetName();
			return FString::Printf(TEXT("%s %s / %s"), *Name, *PropText(P, X, k), *PropText(P, Y, k));
		}
		return FString();
	}

	// The first field that differs between the two peers' states at one frame ("" = bit-identical). Reflection over FJamSkaterState and FJamPuckState,
	// so a field a lane adds is compared without a change here.
	FString FirstDiff(const FSnap& X, const FSnap& Y)
	{
		if (X.Phase != Y.Phase) return FString::Printf(TEXT("sim.Phase %d / %d"), X.Phase, Y.Phase);
		if (X.Score0 != Y.Score0 || X.Score1 != Y.Score1) return FString::Printf(TEXT("sim.Score %d:%d / %d:%d"), X.Score0, X.Score1, Y.Score0, Y.Score1);
		if (X.Timer != Y.Timer) return FString::Printf(TEXT("sim.TimerLeft %d / %d"), X.Timer, Y.Timer);
		if (X.Rng != Y.Rng) return FString::Printf(TEXT("sim.Rng %016llx / %016llx"), static_cast<unsigned long long>(X.Rng), static_cast<unsigned long long>(Y.Rng));
		if (X.Skaters.Num() != Y.Skaters.Num()) return FString::Printf(TEXT("sim.Skaters.Num %d / %d"), X.Skaters.Num(), Y.Skaters.Num());
		for (int32 i = 0; i < X.Skaters.Num(); ++i)
		{
			for (TFieldIterator<FProperty> It(FJamSkaterState::StaticStruct()); It; ++It)
			{
				const FString D = FieldDiff(*It, &X.Skaters[i], &Y.Skaters[i], FString::Printf(TEXT("skater[%d]."), i));
				if (!D.IsEmpty()) return D;
			}
		}
		for (TFieldIterator<FProperty> It(FJamPuckState::StaticStruct()); It; ++It)
		{
			const FString D = FieldDiff(*It, &X.Puck, &Y.Puck, TEXT("puck."));
			if (!D.IsEmpty()) return D;
		}
		if (X.SimH != Y.SimH) return FString::Printf(TEXT("sim.StateHash (unreflected sim state: modifiers, faceoff, rule flags) %016llx / %016llx"), static_cast<unsigned long long>(X.SimH), static_cast<unsigned long long>(Y.SimH));
		if (X.SimLockH != Y.SimLockH) return FString::Printf(TEXT("sim.LockstepStateHash (unreflected cross-frame state: MoveHist, BotDrawFrame) %016llx / %016llx"), static_cast<unsigned long long>(X.SimLockH), static_cast<unsigned long long>(Y.SimLockH));
		if (X.FxH != Y.FxH) return FString::Printf(TEXT("effects.StateHash %016llx / %016llx"), static_cast<unsigned long long>(X.FxH), static_cast<unsigned long long>(Y.FxH));
		if (X.RulesH != Y.RulesH) return FString::Printf(TEXT("rules.StateHash %016llx / %016llx"), static_cast<unsigned long long>(X.RulesH), static_cast<unsigned long long>(Y.RulesH));
		if (X.ZonesH != Y.ZonesH) return FString::Printf(TEXT("zones.StateHash %016llx / %016llx"), static_cast<unsigned long long>(X.ZonesH), static_cast<unsigned long long>(Y.ZonesH));
		return FString();
	}

	FString DumpSnap(const FSnap& S)
	{
		FString Out = FString::Printf(TEXT("lock=%016llx sim=%016llx fx=%016llx rules=%016llx zones=%016llx phase=%d score=%d:%d timer=%d rng=%016llx\n"),
			static_cast<unsigned long long>(S.Lock), static_cast<unsigned long long>(S.SimH), static_cast<unsigned long long>(S.FxH), static_cast<unsigned long long>(S.RulesH),
			static_cast<unsigned long long>(S.ZonesH), S.Phase, S.Score0, S.Score1, S.Timer, static_cast<unsigned long long>(S.Rng));
		for (int32 i = 0; i < S.Skaters.Num(); ++i)
		{
			FString T;
			FJamSkaterState::StaticStruct()->ExportText(T, &S.Skaters[i], nullptr, nullptr, PPF_None, nullptr);
			Out += FString::Printf(TEXT("skater[%d] %s\n"), i, *T);
		}
		FString P;
		FJamPuckState::StaticStruct()->ExportText(P, &S.Puck, nullptr, nullptr, PPF_None, nullptr);
		return Out + TEXT("puck ") + P + TEXT("\n");
	}

	// ---- options ----

	struct FOptions
	{
		int32 SeedFirst = 1, Seeds = 200, Frames = 3600;
		FString Profiles;
		FString OutFile;
		FString Cut = TEXT("hockey.regular");
		bool bSprint = true;   // lane A104: -SoakSprint=0 = A56's playtesters without Shift
		bool bSeatGuest = true;   // lane A110: -SoakSeatGuest=0 = A104's bot seat for the guest
	};

	FOptions ReadOptions(int32 DefaultSeeds)
	{
		FOptions O;
		O.Seeds = DefaultSeeds;
		const TCHAR* Cmd = FCommandLine::Get();
		FParse::Value(Cmd, TEXT("SoakSeedFirst="), O.SeedFirst);
		FParse::Value(Cmd, TEXT("SoakSeeds="), O.Seeds);
		FParse::Value(Cmd, TEXT("SoakFrames="), O.Frames);
		FParse::Value(Cmd, TEXT("SoakProfiles="), O.Profiles, false);
		FParse::Value(Cmd, TEXT("SoakOut="), O.OutFile, false);
		FParse::Value(Cmd, TEXT("SoakCut="), O.Cut);
		FParse::Bool(Cmd, TEXT("SoakSprint="), O.bSprint);
		FParse::Bool(Cmd, TEXT("SoakSeatGuest="), O.bSeatGuest);
		if (O.OutFile.IsEmpty()) O.OutFile = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("SoakLockstep"), FString::Printf(TEXT("soak-%s.jsonl"), *FDateTime::Now().ToString(TEXT("%Y%m%d-%H%M%S"))));
		return O;
	}

	void Emit(const FOptions& O, const FString& Json)
	{
		UE_LOG(LogJamNet, Display, TEXT("SOAKLOCK %s"), *Json);
		FFileHelper::SaveStringToFile(Json + TEXT("\n"), *O.OutFile, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM, &IFileManager::Get(), FILEWRITE_Append);
	}

	UJamCartridge* Load(FAutomationTestBase& T, const FString& Name)
	{
		TArray<FString> Errors;
		UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(Name), Errors);
		for (const FString& E : Errors) T.AddError(E);
		return C;
	}

	const TCHAR* EdgeName(int32 Bit)
	{
		switch (1 << Bit)
		{
		case FJamNetQuantInput::Shoot: return TEXT("shoot");
		case FJamNetQuantInput::Check: return TEXT("check");
		case FJamNetQuantInput::Pass: return TEXT("pass");
		case FJamNetQuantInput::Poke: return TEXT("poke");
		case FJamNetQuantInput::Lift: return TEXT("lift");
		case FJamNetQuantInput::Deke: return TEXT("deke");
		case FJamNetQuantInput::Spin: return TEXT("spin");
		case FJamNetQuantInput::Ability: return TEXT("ability");   // lane A104
		default: return TEXT("bit");
		}
	}

	// ---- one joined match ----

	struct FMatch
	{
		uint64 Seed = 0;
		FString Profile;
		int64 FramesA = 0, FramesB = 0, Ticks = 0;
		bool bDesyncDriver = false, bLost = false, bFinalEqual = false;
		int64 DriverDesyncFrame = -1;
		int64 FirstDiffFrame = -1;     // first frame any compared state differed (bit-exact)
		FString FirstDiffField;
		int64 FirstLockHashDiff = -1;  // first frame the lockstep hash itself differed (what the driver can see)
		int32 AgreedA = 0, AgreedB = 0, Expected = 0;
		int32 StallsA = 0, StallsB = 0, MaxStall = 0, MaxGap = 0;
		int32 Dropped = 0, Blacked = 0, Duplicated = 0, Reordered = 0, Twice = 0, MaxRun = 0, Corrupted = 0;
		int32 EdgePressed[16] = {}, EdgeLost[16] = {};
		int64 FirstEdgeLost = -1;
		int32 Spins = 0;
		int32 Verbs[3] = {};   // lane A104: sprint frames, pass-held frames, ability presses applied
		int32 ReverseBrakes = 0, AbilitiesUsed = 0;   // lane A104: the sims' own counters (peer A)
		int32 GuestAbilitiesUsed = 0;   // lane A110: the guest seat's share of AbilitiesUsed
		int32 Score0 = 0, Score1 = 0;
		double WallS = 0.0;
		FString DumpPath;
	};

	FString MatchJson(const FMatch& M)
	{
		FString Edges;
		for (int32 b = 0; b < 16; ++b)
		{
			if (!M.EdgePressed[b]) continue;
			Edges += FString::Printf(TEXT("%s\"%s\":[%d,%d]"), Edges.IsEmpty() ? TEXT("") : TEXT(","), EdgeName(b), M.EdgePressed[b], M.EdgeLost[b]);
		}
		return FString::Printf(TEXT("{\"schema\":\"thirdjam.soaklock/1\",\"profile\":\"%s\",\"seed\":%llu,\"frames\":[%lld,%lld],\"ticks\":%lld,\"desync\":%s,\"driver_desync_frame\":%lld,")
			TEXT("\"first_diff_frame\":%lld,\"first_diff_field\":\"%s\",\"first_lockhash_diff\":%lld,\"lost\":%s,\"final_equal\":%s,\"agreed\":[%d,%d],\"expected\":%d,")
			TEXT("\"stalls\":[%d,%d],\"max_stall\":%d,\"max_gap\":%d,\"dropped\":%d,\"blacked\":%d,\"dup\":%d,\"reordered\":%d,\"twice\":%d,\"max_loss_run\":%d,\"corrupted\":%d,")
			TEXT("\"edges\":{%s},\"first_edge_lost\":%lld,\"spins_applied\":%d,\"verbs_applied\":{\"sprint_frames\":%d,\"pass_held_frames\":%d,\"ability\":%d},\"reverse_brakes\":%d,\"abilities_used\":%d,\"guest_abilities_used\":%d,\"score\":\"%d:%d\",\"wall_s\":%.2f%s}"),
			*M.Profile, static_cast<unsigned long long>(M.Seed), M.FramesA, M.FramesB, M.Ticks, M.bDesyncDriver ? TEXT("true") : TEXT("false"), M.DriverDesyncFrame,
			M.FirstDiffFrame, *M.FirstDiffField.ReplaceCharWithEscapedChar().Replace(TEXT("\""), TEXT("'")), M.FirstLockHashDiff, M.bLost ? TEXT("true") : TEXT("false"),
			M.bFinalEqual ? TEXT("true") : TEXT("false"), M.AgreedA, M.AgreedB, M.Expected,
			M.StallsA, M.StallsB, M.MaxStall, M.MaxGap, M.Dropped, M.Blacked, M.Duplicated, M.Reordered, M.Twice, M.MaxRun, M.Corrupted,
			*Edges, M.FirstEdgeLost, M.Spins, M.Verbs[0], M.Verbs[1], M.Verbs[2], M.ReverseBrakes, M.AbilitiesUsed, M.GuestAbilitiesUsed, M.Score0, M.Score1, M.WallS,
			M.DumpPath.IsEmpty() ? TEXT("") : *FString::Printf(TEXT(",\"dump\":\"%s\""), *M.DumpPath.Replace(TEXT("\\"), TEXT("/"))));
	}

	FMatch RunMatch(FAutomationTestBase& T, const FOptions& O, const UJamCartridge& C, uint64 Seed, const FFaults& Fa, FTiming& Tm)
	{
		FMatch M;
		M.Seed = Seed;
		M.Profile = (O.Cut == TEXT("hockey.regular") ? Fa.Name : Fa.Name + TEXT("@") + O.Cut)   // the brief's cut and length are unmarked; others carry theirs
			+ (O.Frames == 3600 ? FString() : FString::Printf(TEXT("/%df"), O.Frames));
		const double T0 = FPlatformTime::Seconds();
		FSoakLink Link(Fa, Seed * 7919 + 17);
		UJamSim* Probe = NewObject<UJamSim>();
		Probe->Init(C.Game, Seed);
		const int32 Host = Probe->GetHumanIndex(), Guest = GuestSlot(*Probe);
		if (Guest < 0 || Host < 0 || Guest == Host) { T.AddError(TEXT("no guest seat")); return M; }
		// The udp profile: two real sockets on loopback (the game's own FJamUdpLink: SendTo / RecvFrom, the peer filter, the 512-byte scratch).
		TUniquePtr<FJamUdpLink> UA, UB;
		IJamLink* LA = &Link.A;
		IJamLink* LB = &Link.B;
		if (Fa.bUdp)
		{
			UA = MakeUnique<FJamUdpLink>(); UB = MakeUnique<FJamUdpLink>();
			if (UA->Bind(0) < 0 || UB->Bind(0) < 0) { T.AddError(TEXT("udp: bind failed")); return M; }
			UA->SetPeer(0x7F000001u, UB->GetBoundPort());
			UB->SetPeer(0x7F000001u, UA->GetBoundPort());
			LA = UA.Get(); LB = UB.Get();
		}
		FPeer A, B;
		A.bSprintOn = B.bSprintOn = O.bSprint;
		A.bSeatGuest = B.bSeatGuest = O.bSeatGuest;
		A.Init(T, C, Seed, *LA, Host, Guest, JamAI::EPlaytester::Regular);
		B.Init(T, C, Seed, *LB, Guest, Host, JamAI::EPlaytester::Good);
		B.InjectFrame = Fa.InjectFrame; B.InjectSkater = Guest;   // lane A120
		double NextAt = FPlatformTime::Seconds();
		const int32 Target = O.Frames;
		M.Expected = Target / JamNet::HashInterval;
		const int64 MaxTicks = static_cast<int64>(Target) * 6 + 4000;
		int64 Tick = 0;
		bool bDone = false;
		int32 Flush = 0;
		for (; Tick < MaxTicks; ++Tick)
		{
			if (Fa.bUdp) { NextAt += Fa.UdpTickUs * 1e-6; while (FPlatformTime::Seconds() < NextAt) FPlatformProcess::YieldThread(); }
			Link.Advance();
			A.Pump(Tm);
			B.Pump(Tm);
			if (bDone)
			{
				// Both reached the target: keep pumping until the last checkpoint is compared on both sides (or 600 ticks).
				if (A.Driver->IsDesynced() || B.Driver->IsDesynced()) break;   // the last checkpoint disagreed: the card is up, nothing to flush
				if ((A.Driver->GetAgreedCheckpoints() >= M.Expected && B.Driver->GetAgreedCheckpoints() >= M.Expected) || ++Flush > 600) break;
				continue;
			}
			if (A.Sim->GetFrame() < Target) A.Step(Tm);
			if (B.Sim->GetFrame() < Target && (Tick % Fa.BEvery) == 0) B.Step(Tm);
			// Bit-exact comparison of every frame both peers have stepped.
			for (auto It = A.Snaps.CreateIterator(); It; ++It)
			{
				FSnap* Bs = B.Snaps.Find(It.Key());
				if (!Bs) continue;
				if (M.FirstDiffFrame < 0)
				{
					const FString D = FirstDiff(It.Value(), *Bs);
					if (!D.IsEmpty())
					{
						M.FirstDiffFrame = It.Key();
						M.FirstDiffField = D;
						const FString Dir = FPaths::Combine(FPaths::GetPath(O.OutFile), TEXT("dumps"));
						M.DumpPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(Dir, FString::Printf(TEXT("%s-seed%llu-f%lld.txt"), *Fa.Name, static_cast<unsigned long long>(Seed), It.Key())));
						FFileHelper::SaveStringToFile(FString::Printf(TEXT("profile %s seed %llu frame %lld first diff: %s\n--- host (A)\n%s--- guest (B)\n%s"), *Fa.Name,
							static_cast<unsigned long long>(Seed), It.Key(), *D, *DumpSnap(It.Value()), *DumpSnap(*Bs)), *M.DumpPath);
					}
				}
				if (M.FirstLockHashDiff < 0 && It.Value().Lock != Bs->Lock) M.FirstLockHashDiff = It.Key();
				B.Snaps.Remove(It.Key());
				It.RemoveCurrent();
			}
			if (A.Driver->IsDesynced() || B.Driver->IsDesynced() || A.Driver->IsLost() || B.Driver->IsLost()) break;
			if (A.Sim->GetFrame() >= Target && B.Sim->GetFrame() >= Target) bDone = true;
		}
		M.WallS = FPlatformTime::Seconds() - T0;
		M.Ticks = Tick;
		M.FramesA = A.Sim->GetFrame(); M.FramesB = B.Sim->GetFrame();
		M.bDesyncDriver = A.Driver->IsDesynced() || B.Driver->IsDesynced();
		M.DriverDesyncFrame = A.Driver->IsDesynced() ? A.Driver->GetDesyncFrame() : B.Driver->GetDesyncFrame();
		M.bLost = A.Driver->IsLost() || B.Driver->IsLost();
		M.bFinalEqual = M.FramesA == M.FramesB && A.LockHash() == B.LockHash();
		M.AgreedA = A.Driver->GetAgreedCheckpoints(); M.AgreedB = B.Driver->GetAgreedCheckpoints();
		M.StallsA = A.Stalls; M.StallsB = B.Stalls;
		M.MaxStall = FMath::Max(A.Driver->GetMaxStallTicks(), B.Driver->GetMaxStallTicks());
		M.MaxGap = FMath::Max(A.Driver->GetMaxFrameGap(), B.Driver->GetMaxFrameGap());
		M.Dropped = Link.A.Dropped + Link.B.Dropped; M.Blacked = Link.A.Blacked + Link.B.Blacked; M.Duplicated = Link.A.Duplicated + Link.B.Duplicated;
		M.Reordered = Link.A.Reordered + Link.B.Reordered; M.Twice = Link.A.Twice + Link.B.Twice; M.MaxRun = FMath::Max(Link.A.MaxRun, Link.B.MaxRun);
		M.Corrupted = Link.B.Corrupted;
		for (FPeer* P : { &A, &B })
			for (int32 b = 0; b < 16; ++b)
			{
				P->EdgeLost[b] = P->EdgePressed[b] - P->EdgeApplied[b] - P->Driver->GetPressesInFlight(static_cast<uint16>(1 << b)) - P->Driver->GetPressesHeld(static_cast<uint16>(1 << b));   // lane A140: a held key's absorbed repeats are no presses
				if (P->EdgeLost[b] != 0 && P->FirstEdgeLostFrame < 0) P->FirstEdgeLostFrame = P->Driver->GetFrame();   // lane A140: the frame it was found at
			}
		for (int32 b = 0; b < 16; ++b) { M.EdgePressed[b] = A.EdgePressed[b] + B.EdgePressed[b]; M.EdgeLost[b] = A.EdgeLost[b] + B.EdgeLost[b]; }
		M.FirstEdgeLost = A.FirstEdgeLostFrame >= 0 ? A.FirstEdgeLostFrame : B.FirstEdgeLostFrame;
		M.Spins = A.Spins;
		for (int32 v = 0; v < 3; ++v) M.Verbs[v] = A.Verbs[v];
		for (const FJamSkaterState& Sk : A.Sim->GetSkaters()) { M.ReverseBrakes += Sk.ReverseBrakes; M.AbilitiesUsed += Sk.AbilitiesUsed; }
		M.GuestAbilitiesUsed = A.Sim->GetSkaters().IsValidIndex(Guest) ? A.Sim->GetSkaters()[Guest].AbilitiesUsed : 0;
		M.Score0 = A.Sim->GetScore(0); M.Score1 = A.Sim->GetScore(1);
		return M;
	}

	struct FAgg
	{
		int32 Matches = 0, Desyncs = 0, Lost = 0, Unfinished = 0, Missed = 0, EdgeLossMatches = 0, Drifts = 0;
		int64 Frames = 0, Ticks = 0, Stalls = 0;
		int32 MaxStall = 0, MaxGap = 0, MaxRun = 0;
		int64 Dropped = 0, Dup = 0, Reordered = 0;
		int64 EdgePressed[16] = {}, EdgeLost[16] = {};
		int64 Spins = 0;
		int64 Verbs[3] = {}, ReverseBrakes = 0, AbilitiesUsed = 0;   // lane A104
		int64 GuestAbilitiesUsed = 0;   // lane A110
		TArray<FString> DesyncLines;
		double Wall = 0.0;

		void Add(const FMatch& M)
		{
			++Matches;
			if (M.bDesyncDriver || M.FirstDiffFrame >= 0) { ++Desyncs; DesyncLines.Add(FString::Printf(TEXT("seed %llu: first diff frame %lld (%s), lockstep hash differs from %lld, driver %s at %lld"),
				static_cast<unsigned long long>(M.Seed), M.FirstDiffFrame, *M.FirstDiffField, M.FirstLockHashDiff, M.bDesyncDriver ? TEXT("desynced") : TEXT("did not see it"), M.DriverDesyncFrame)); }
			if (M.FirstDiffFrame >= 0 && M.FirstLockHashDiff < 0) ++Drifts;
			if (M.bLost) ++Lost;
			if (!M.bLost && !M.bDesyncDriver && (M.FramesA < 0 || M.AgreedA < M.Expected || M.AgreedB < M.Expected)) ++Missed;
			Frames += FMath::Min(M.FramesA, M.FramesB); Ticks += M.Ticks; Stalls += M.StallsA + M.StallsB;
			MaxStall = FMath::Max(MaxStall, M.MaxStall); MaxGap = FMath::Max(MaxGap, M.MaxGap); MaxRun = FMath::Max(MaxRun, M.MaxRun);
			Dropped += M.Dropped + M.Blacked; Dup += M.Duplicated; Reordered += M.Reordered;
			bool bEdge = false;
			for (int32 b = 0; b < 16; ++b) { EdgePressed[b] += M.EdgePressed[b]; EdgeLost[b] += M.EdgeLost[b]; bEdge |= M.EdgeLost[b] > 0; }
			EdgeLossMatches += bEdge ? 1 : 0;
			Spins += M.Spins;
			for (int32 v = 0; v < 3; ++v) Verbs[v] += M.Verbs[v];
			ReverseBrakes += M.ReverseBrakes; AbilitiesUsed += M.AbilitiesUsed; GuestAbilitiesUsed += M.GuestAbilitiesUsed;
			Wall += M.WallS;
		}
		FString Edges() const
		{
			FString S;
			for (int32 b = 0; b < 16; ++b) if (EdgePressed[b]) S += FString::Printf(TEXT(" %s %lld/%lld"), EdgeName(b), EdgeLost[b], EdgePressed[b]);
			return S;
		}
	};

	double Micro(uint64 Cycles) { return FPlatformTime::ToMilliseconds64(Cycles) * 1000.0; }

	FString TimingLine(const TCHAR* What, TArray<uint32>& V)
	{
		if (!V.Num()) return FString::Printf(TEXT("%s: none"), What);
		uint64 Sum = 0; for (uint32 X : V) Sum += X;
		V.Sort();
		return FString::Printf(TEXT("%s: n=%d mean %.2f us, p50 %.2f us, p99 %.2f us, max %.1f us"), What, V.Num(), Micro(Sum) / V.Num(), Micro(V[V.Num() / 2]),
			Micro(V[FMath::Min(V.Num() - 1, static_cast<int32>(V.Num() * 0.99))]), Micro(V.Last()));
	}

	FFaults Profile(const FString& Name)
	{
		FFaults F; F.Name = Name;
		if (Name == TEXT("clean")) { }
		else if (Name == TEXT("loss2")) { F.Loss = 0.02; }
		else if (Name == TEXT("loss10")) { F.Loss = 0.10; }
		else if (Name == TEXT("loss30")) { F.Loss = 0.30; }
		else if (Name == TEXT("jitter0-6")) { F.DelayMin = 0; F.DelayMax = 6; }
		else if (Name == TEXT("dup20")) { F.Dup = 0.20; F.DelayMin = 0; F.DelayMax = 2; }
		else if (Name == TEXT("loss10-jitter0-6-dup10")) { F.Loss = 0.10; F.DelayMin = 0; F.DelayMax = 6; F.Dup = 0.10; }
		else if (Name == TEXT("loss30-jitter0-6-dup10")) { F.Loss = 0.30; F.DelayMin = 0; F.DelayMax = 6; F.Dup = 0.10; }
		else if (Name == TEXT("udp-loopback")) { F.bUdp = true; }   // real sockets, 127.0.0.1, ticks paced at 250 us
		else if (Name == TEXT("slow30")) { F.BEvery = 2; }   // the guest's machine at 30 Hz, LAN link
		else if (Name == TEXT("slow30-loss10-jitter0-6")) { F.BEvery = 2; F.Loss = 0.10; F.DelayMin = 0; F.DelayMax = 6; }
		else if (Name == TEXT("blackout120")) { F.BlackoutAt = 1200; F.BlackoutTicks = 120; }   // 2 s without a datagram: inside the 3 s stall limit
		else if (Name == TEXT("inject-movehist7")) { F.InjectFrame = 300; F.bExpectDesync = true; }   // lane A120: the field-compare proof (one element of a C array)
		else if (Name == TEXT("blackout240")) { F.BlackoutAt = 1200; F.BlackoutTicks = 240; F.bExpectLost = true; }   // 4 s: the "connection lost" card
		else { F.Name = TEXT("?") + Name; }
		return F;
	}

	void RunProfile(FAutomationTestBase& T, const FOptions& O, const UJamCartridge& C, const FFaults& Fa, FTiming& Tm, FAgg& Agg)
	{
		for (int32 s = 0; s < O.Seeds; ++s)
		{
			const uint64 Seed = static_cast<uint64>(O.SeedFirst + s);
			const FMatch M = RunMatch(T, O, C, Seed, Fa, Tm);
			Emit(O, MatchJson(M));
			Agg.Add(M);
			const bool bDiff = M.bDesyncDriver || M.FirstDiffFrame >= 0;
			if (bDiff && !Fa.bExpectDesync) T.AddError(FString::Printf(TEXT("%s seed %llu: DESYNC first differing frame %lld field %s (lockstep hash from %lld, driver checkpoint %lld) dump %s"), *Fa.Name,
				static_cast<unsigned long long>(Seed), M.FirstDiffFrame, *M.FirstDiffField, M.FirstLockHashDiff, M.DriverDesyncFrame, *M.DumpPath));
			if (M.bLost != Fa.bExpectLost) T.AddError(FString::Printf(TEXT("%s seed %llu: %s (frames %lld/%lld, max stall %d ticks, longest loss run %d)"), *Fa.Name, static_cast<unsigned long long>(Seed),
				M.bLost ? TEXT("CONNECTION LOST") : TEXT("expected the connection-lost end, the match went on"), M.FramesA, M.FramesB, M.MaxStall, M.MaxRun));
			if (!M.bLost && !bDiff && (M.FramesA != O.Frames || M.FramesB != O.Frames || M.AgreedA != M.Expected || M.AgreedB != M.Expected || !M.bFinalEqual))
				T.AddError(FString::Printf(TEXT("%s seed %llu: unfinished frames %lld/%lld agreed %d/%d of %d final equal %d"), *Fa.Name, static_cast<unsigned long long>(Seed), M.FramesA, M.FramesB, M.AgreedA, M.AgreedB, M.Expected, M.bFinalEqual ? 1 : 0));
			for (int32 b = 0; b < 16; ++b)
				if (M.EdgeLost[b]) T.AddError(FString::Printf(TEXT("%s seed %llu: %d of %d %s presses never reached the sim (first at frame %lld)"), *Fa.Name, static_cast<unsigned long long>(Seed), M.EdgeLost[b], M.EdgePressed[b], EdgeName(b), M.FirstEdgeLost));
		}
		const double Speed = Agg.Ticks > 0 ? static_cast<double>(Agg.Frames) / Agg.Ticks : 0.0;
		const FString Sum = FString::Printf(TEXT("{\"schema\":\"thirdjam.soaklock.summary/1\",\"profile\":\"%s\",\"seeds\":\"%d..%d\",\"matches\":%d,\"desyncs\":%d,\"silent_drifts\":%d,\"lost\":%d,\"missed_checkpoints\":%d,")
			TEXT("\"frames\":%lld,\"ticks\":%lld,\"speed\":%.4f,\"stall_attempts\":%lld,\"max_stall\":%d,\"max_gap\":%d,\"max_loss_run\":%d,\"dropped\":%lld,\"dup\":%lld,\"reordered\":%lld,")
			TEXT("\"edge_loss_matches\":%d,\"edges\":\"%s\",\"spins_applied\":%lld,\"verbs_applied\":{\"sprint_frames\":%lld,\"pass_held_frames\":%lld,\"ability\":%lld},\"reverse_brakes\":%lld,\"abilities_used\":%lld,\"guest_abilities_used\":%lld,\"wall_s\":%.1f}"),
			*(O.Cut == TEXT("hockey.regular") ? Fa.Name : Fa.Name + TEXT("@") + O.Cut), O.SeedFirst, O.SeedFirst + O.Seeds - 1, Agg.Matches, Agg.Desyncs, Agg.Drifts, Agg.Lost, Agg.Missed, Agg.Frames, Agg.Ticks, Speed, Agg.Stalls, Agg.MaxStall, Agg.MaxGap, Agg.MaxRun,
			Agg.Dropped, Agg.Dup, Agg.Reordered, Agg.EdgeLossMatches, *Agg.Edges().TrimStart(), Agg.Spins, Agg.Verbs[0], Agg.Verbs[1], Agg.Verbs[2], Agg.ReverseBrakes, Agg.AbilitiesUsed, Agg.GuestAbilitiesUsed, Agg.Wall);
		Emit(O, Sum);
		T.AddInfo(Sum);
		for (const FString& L : Agg.DesyncLines) T.AddInfo(Fa.Name + TEXT(" ") + L);
	}

	void EmitTiming(FAutomationTestBase& T, const FOptions& O, FTiming& Tm, const FString& Label)
	{
		const FString P = TimingLine(TEXT("Pump (receive + deserialize + hash compare + serialize + send)"), Tm.Pump);
		const FString Tr = TimingLine(TEXT("TryStep (quantise + gate + apply)"), Tm.Try);
		const FString S = TimingLine(TEXT("sim frame (brain + UJamSim::Step + zones + effects), for scale"), Tm.Sim);
		for (const FString& L : { P, Tr, S })
		{
			T.AddInfo(Label + TEXT(" ") + L);
			Emit(O, FString::Printf(TEXT("{\"schema\":\"thirdjam.soaklock.timing/1\",\"label\":\"%s\",\"line\":\"%s\"}"), *Label, *L));
		}
	}
}

using namespace JamNetSoak;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNetSoakClean, "Bar.Net.SoakClean", JamNetSoak::Flags)
bool FJamNetSoakClean::RunTest(const FString& Parameters)
{
	const FOptions O = ReadOptions(200);
	UJamCartridge* C = Load(*this, O.Cut);
	if (!C) return false;
	AddInfo(FString::Printf(TEXT("soak: %s at pro, regular (host) vs good (guest), seeds %d..%d, %d frames, protocol %d, out %s"), *O.Cut, O.SeedFirst, O.SeedFirst + O.Seeds - 1, O.Frames, JamNet::Version, *O.OutFile));
	FTiming Tm;
	FAgg Agg;
	RunProfile(*this, O, *C, Profile(TEXT("clean")), Tm, Agg);
	EmitTiming(*this, O, Tm, TEXT("clean"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNetSoakLossy, "Bar.Net.SoakLossy", JamNetSoak::Flags)
bool FJamNetSoakLossy::RunTest(const FString& Parameters)
{
	FOptions O = ReadOptions(50);
	UJamCartridge* C = Load(*this, O.Cut);
	if (!C) return false;
	TArray<FString> Names;
	(O.Profiles.IsEmpty() ? FString(TEXT("loss2,loss10,loss30,jitter0-6,dup20,loss10-jitter0-6-dup10,loss30-jitter0-6-dup10,blackout120,blackout240")) : O.Profiles).ParseIntoArray(Names, TEXT(","));
	FTiming Tm;
	for (const FString& N : Names)
	{
		const FFaults F = Profile(N.TrimStartAndEnd());
		if (F.Name.StartsWith(TEXT("?"))) { AddError(FString::Printf(TEXT("unknown profile %s"), *N)); continue; }
		FAgg Agg;
		RunProfile(*this, O, *C, F, Tm, Agg);
	}
	EmitTiming(*this, O, Tm, TEXT("lossy"));
	return true;
}

namespace JamNetSoak
{
	// A host driver alone, fed crafted datagrams by a fake guest for Ticks engine ticks.
	struct FHostProbe
	{
		bool bHeard = false, bLost = false, bDesync = false;
		int64 Frame = 0, LostAtTick = -1;
		int32 ReadOk = 0, ReadRefused = 0;
	};

	FHostProbe FeedHost(TFunctionRef<void(int64 Tick, TArray<uint8>& Out)> Make, int32 Ticks)
	{
		FFaults F; F.Name = TEXT("probe"); F.DelayMin = F.DelayMax = 0;
		FSoakLink Link(F, 1);
		TStrongObjectPtr<UJamLockstepDriver> Host(NewObject<UJamLockstepDriver>());
		Host->Start(&Link.A, JamNet::InputDelay);
		FHostProbe R;
		for (int64 t = 0; t < Ticks; ++t)
		{
			Link.Advance();
			TArray<uint8> Bytes;
			Make(t, Bytes);
			if (Bytes.Num())
			{
				FJamNetInputPacket P;
				if (FJamNetInputPacket::Read(Bytes, P)) ++R.ReadOk; else ++R.ReadRefused;
				Link.B.Send(Bytes);
			}
			Host->Pump();
			FJamInput L, Rm;
			Host->TryStep(FJamInput(), L, Rm);
			if (Host->IsLost() && R.LostAtTick < 0) R.LostAtTick = t;
		}
		R.bHeard = Host->HasHeardPeer(); R.bLost = Host->IsLost(); R.bDesync = Host->IsDesynced(); R.Frame = Host->GetFrame();
		return R;
	}

	// A well-formed protocol-5 input packet from the guest's side for its frame Frame (window ending at Frame + Delay).
	void GuestPacket(int64 Frame, TArray<uint8>& Out, uint16 InFlags = 0)
	{
		FJamNetInputPacket P;
		const int64 Next = Frame + JamNet::InputDelay;
		P.FirstFrame = FMath::Max<int64>(0, Next - JamNet::Window);
		for (int64 F = P.FirstFrame; F < Next; ++F) { FJamNetQuantInput Q; Q.Flags = InFlags; P.Inputs.Add(Q); }
		P.SenderFrame = Frame;
		P.Write(Out);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNetSoakMismatch, "Bar.Net.SoakMismatch", JamNetSoak::Flags)
bool FJamNetSoakMismatch::RunTest(const FString& Parameters)
{
	FOptions O = ReadOptions(20);
	// The corrupt-byte profile is meant to desync: the driver's own error line is expected there (any count).
	AddExpectedError(TEXT("JAMNET desync"), EAutomationExpectedErrorFlags::Contains, 0);
	auto Row = [&](const TCHAR* Case, const FString& Does, const FString& Reason, bool bOk)
	{
		const FString J = FString::Printf(TEXT("{\"schema\":\"thirdjam.soaklock.mismatch/1\",\"case\":\"%s\",\"host_does\":\"%s\",\"reason_shown\":\"%s\",\"crash\":false,\"as_wanted\":%s}"),
			Case, *Does.Replace(TEXT("\""), TEXT("'")), *Reason.Replace(TEXT("\""), TEXT("'")), bOk ? TEXT("true") : TEXT("false"));
		Emit(O, J);
		AddInfo(J);
	};

	// 1. Protocol 4 against 5, lobby: the guest's HELLO with version byte 4.
	{
		FJamNetLobbyMessage Hello; Hello.Type = JamNet::EType::Hello; Hello.Name = TEXT("Old Build");
		TArray<uint8> Bytes; Hello.Write(Bytes); Bytes[4] = 4;
		FJamNetLobbyMessage M;
		const bool bRead = FJamNetLobbyMessage::Read(Bytes, M);
		const bool bPeek = JamNetPeekType(Bytes) != JamNet::EType::None;
		// UJamLobbySubsystem::TickLobby (Hosting): `if (!FJamNetLobbyMessage::Read(...) || M.Type != Hello) continue;` -> nothing is sent back; the guest
		// resends HELLO every ResendEvery ticks and fails after 3600 lobby ticks with "no answer from the host in 60 s (same network? port forwarded?)".
		Row(TEXT("protocol 4 HELLO to a protocol 5 host (lobby)"), FString::Printf(TEXT("Read=%d PeekType=%s: the datagram is skipped like noise; no WELCOME, no refusal goes back"), bRead ? 1 : 0, bPeek ? TEXT("ours") : TEXT("None")),
			TEXT("host: nothing (stays 'waiting for a friend'); guest after 60 s: 'no answer from the host in 60 s (same network? port forwarded?)'"), !bRead && !bPeek);
		TestFalse(TEXT("a protocol-4 HELLO is not read as a HELLO"), bRead);
	}
	// 2. Protocol 4 input packets reaching a protocol-5 host driver in a match.
	{
		const FHostProbe R = FeedHost([](int64 t, TArray<uint8>& Out) { GuestPacket(t, Out); Out[4] = 4; }, 2000);
		Row(TEXT("protocol 4 input packets to a protocol 5 driver"), FString::Printf(TEXT("all %d refused by Read; peer never heard; frame stays %lld; lost at tick %lld (10 x 180, the not-yet-heard limit)"), R.ReadRefused, R.Frame, R.LostAtTick),
			TEXT("'CONNECTION LOST: no word from <name> for 3 seconds' (after 30 s, since an unheard peer gets 10 x the limit)"), R.ReadOk == 0 && !R.bHeard && R.bLost);
		TestTrue(TEXT("protocol 4 inputs never step the host"), R.Frame == 0 && !R.bHeard);
	}
	// 3. Unknown flag bits on a protocol-5 packet: bit 1024 (the next free bit) and 0x8000.
	for (const uint16 Bit : { static_cast<uint16>(1024), static_cast<uint16>(0x8000) })
	{
		TArray<uint8> Bytes; GuestPacket(10, Bytes, Bit);
		FJamNetInputPacket P;
		const bool bRead = FJamNetInputPacket::Read(Bytes, P);
		const FJamInput In = bRead && P.Inputs.Num() ? P.Inputs[0].ToInput() : FJamInput();
		const bool bAny = In.bShoot || In.bCheck || In.bBrake || In.bShootHeld || In.bPass || In.bPoke || In.bLift || In.bDeke || In.bSprint || In.bSpin;
		const FHostProbe R = FeedHost([Bit](int64 t, TArray<uint8>& Out) { GuestPacket(t, Out, Bit); }, 600);
		Row(*FString::Printf(TEXT("unknown flag bit %u on protocol 5"), Bit), FString::Printf(TEXT("Read=%d, accepted; ToInput drops the bit (any verb set: %d); a driver fed it steps to frame %lld, desync %d"), bRead ? 1 : 0, bAny ? 1 : 0, R.Frame, R.bDesync ? 1 : 0),
			TEXT("none: silently ignored (a sender that applies its own bit would desync at the next checkpoint)"), bRead && !bAny);
		TestFalse(*FString::Printf(TEXT("unknown bit %u sets no verb"), Bit), bAny);
	}
	// 4. Wrong sizes.
	{
		TArray<uint8> Good; GuestPacket(10, Good);
		struct FCase { const TCHAR* Name; TArray<uint8> Bytes; };
		TArray<FCase> Cases;
		Cases.Add({ TEXT("0 bytes"), {} });
		Cases.Add({ TEXT("5 bytes (magic + version)"), TArray<uint8>(Good.GetData(), 5) });
		Cases.Add({ TEXT("6 bytes (header type only)"), TArray<uint8>(Good.GetData(), 6) });
		Cases.Add({ TEXT("26 bytes (header minus the count)"), TArray<uint8>(Good.GetData(), 26) });
		Cases.Add({ TEXT("27 bytes, count 8, no inputs"), TArray<uint8>(Good.GetData(), 27) });
		Cases.Add({ TEXT("one input byte short"), TArray<uint8>(Good.GetData(), Good.Num() - 1) });
		{ TArray<uint8> B = Good; B.Add(0xAB); Cases.Add({ TEXT("one trailing byte"), B }); }
		{ TArray<uint8> B = Good; B.AddZeroed(100); Cases.Add({ TEXT("100 trailing bytes"), B }); }
		{ TArray<uint8> B = Good; B[26] = 255; B.SetNumZeroed(27 + 255 * FJamNetQuantInput::WireBytes); Cases.Add({ TEXT("count 255, full payload (2577 B, over MaxDatagram 512)"), B }); }
		{ TArray<uint8> B = Good; B[26] = 255; Cases.Add({ TEXT("count 255, 8 inputs of payload"), B }); }
		{ TArray<uint8> B = Good; B[26] = 0; B.SetNum(27); Cases.Add({ TEXT("count 0"), B }); }
		{ TArray<uint8> B = Good; uint32 Huge = 0xFFFFFFF0u; FMemory::Memcpy(&B[6], &Huge, 4); Cases.Add({ TEXT("first frame 0xFFFFFFF0"), B }); }
		for (const FCase& K : Cases)
		{
			FJamNetInputPacket P;
			const bool bRead = FJamNetInputPacket::Read(K.Bytes, P);
			FJamNetLobbyMessage M;
			const bool bLobby = FJamNetLobbyMessage::Read(K.Bytes, M);
			// The same datagram every tick into a host driver (no good packets): it must neither crash nor step.
			const TArray<uint8> Bytes = K.Bytes;
			const FHostProbe R = FeedHost([&Bytes](int64 t, TArray<uint8>& Out) { Out = Bytes; }, 300);
			Row(K.Name, FString::Printf(TEXT("Read=%d (inputs %d) lobby Read=%d; host fed it 300 ticks: heard %d frame %lld"), bRead ? 1 : 0, bRead ? P.Inputs.Num() : -1, bLobby ? 1 : 0, R.bHeard ? 1 : 0, R.Frame),
				bRead ? TEXT("none: accepted") : TEXT("none: skipped like noise (the match side shows 'connection lost' if nothing else comes)"), true);
		}
	}
	// 5. Fuzz: 20 000 mutations of a good packet (1..4 random bytes, random truncation or extension) through Read, PeekType, lobby Read and a live driver.
	{
		FJamRandom Rng(56);
		TArray<uint8> Good; GuestPacket(40, Good, FJamNetQuantInput::Spin);
		int32 Ok = 0, Refused = 0;
		FFaults F; F.DelayMin = F.DelayMax = 0;
		FSoakLink Link(F, 2);
		TStrongObjectPtr<UJamLockstepDriver> Host(NewObject<UJamLockstepDriver>());
		Host->Start(&Link.A, JamNet::InputDelay);
		for (int32 i = 0; i < 20000; ++i)
		{
			TArray<uint8> B = Good;
			const int32 Kind = Rng.NextInt(4);
			if (Kind == 0) B.SetNum(Rng.NextInt(B.Num() + 1));
			else if (Kind == 1) for (int32 k = 0, n = 1 + Rng.NextInt(16); k < n; ++k) B.Add(static_cast<uint8>(Rng.NextU32()));
			for (int32 k = 0, n = 1 + Rng.NextInt(4); k < n && B.Num(); ++k) B[Rng.NextInt(B.Num())] = static_cast<uint8>(Rng.NextU32());
			FJamNetInputPacket P; FJamNetLobbyMessage M;
			if (FJamNetInputPacket::Read(B, P)) ++Ok; else ++Refused;
			FJamNetLobbyMessage::Read(B, M);
			JamNetPeekType(B);
			Link.B.Send(B);
			Link.Advance();
			Host->Pump();
			FJamInput L, R;
			Host->TryStep(FJamInput(), L, R);
		}
		Row(TEXT("fuzz: 20000 mutated packets"), FString::Printf(TEXT("%d read as input packets, %d refused; a driver fed all of them ends at frame %lld, lost %d, desync %d"), Ok, Refused, Host->GetFrame(), Host->IsLost() ? 1 : 0, Host->IsDesynced() ? 1 : 0),
			TEXT("none (no crash; mutated packets that keep magic+version+type and a fitting count are applied as inputs)"), true);
	}
	// 6. Live matches with a faulty peer: unknown bit on every input word, and 1 % of B's datagrams with one flipped byte (UDP checksum off).
	{
		UJamCartridge* C = Load(*this, O.Cut);
		if (!C) return false;
		O.Frames = FMath::Min(O.Frames, 3600);
		FTiming Tm;
		FFaults Unknown; Unknown.Name = TEXT("unknown-bit-1024-from-guest"); Unknown.OrFlagsFromB = 1024;
		FFaults Old; Old.Name = TEXT("protocol-4-guest"); Old.VersionFromB = 4; Old.bExpectLost = true;
		FFaults Corrupt; Corrupt.Name = TEXT("corrupt-1pct-from-guest"); Corrupt.Corrupt = 0.01; Corrupt.bExpectDesync = true;
		for (const FFaults* Fp : { &Unknown, &Old, &Corrupt })
		{
			FOptions Oo = O;
			Oo.Seeds = FMath::Min(O.Seeds, 20);
			FAgg Agg;
			int32 Desync = 0, Lost = 0, Diverged = 0;
			FString Fields;
			for (int32 s = 0; s < Oo.Seeds; ++s)
			{
				const FMatch M = RunMatch(*this, Oo, *C, static_cast<uint64>(Oo.SeedFirst + s), *Fp, Tm);
				Emit(Oo, MatchJson(M));
				Agg.Add(M);
				if (M.bDesyncDriver) ++Desync;
				if (M.FirstDiffFrame >= 0) { ++Diverged; if (Fields.Len() < 400) Fields += FString::Printf(TEXT("%s seed %llu f%lld %s (driver %lld); "), Fields.IsEmpty() ? TEXT("") : TEXT(""), static_cast<unsigned long long>(M.Seed), M.FirstDiffFrame, *M.FirstDiffField.Left(60), M.DriverDesyncFrame); }
				if (M.bLost) ++Lost;
			}
			Row(*Fp->Name, FString::Printf(TEXT("%d matches: %d diverged, %d desync cards, %d lost; %s"), Oo.Seeds, Diverged, Desync, Lost, *Fields),
				Fp == &Old ? TEXT("connection lost (no protocol reason)") : Fp == &Corrupt ? TEXT("DESYNC card at the next 60-frame checkpoint") : TEXT("none"), true);
			if (Fp == &Unknown) TestEqual(TEXT("an unknown flag bit from the guest changes nothing on either side"), Diverged, 0);
		}
	}
	return true;
}

#endif

// Lane A120 (track Q1): the soak's field compare covers whole arrays. One element of one C array (the guest's MoveHist[7], the burst's ring) is nudged on
// the guest's machine after frame 300 of a clean regular match: the compare must name that element at that frame (it compared element 0 only before), and
// the lockstep hash must see it at the same frame (300 is a checkpoint), so the driver's DESYNC card comes up on it.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNetSoakFieldArrays, "Jam.Net.SoakFieldArrays", JamNetSoak::Flags)
bool FJamNetSoakFieldArrays::RunTest(const FString& Parameters)
{
	AddExpectedError(TEXT("JAMNET desync"), EAutomationExpectedErrorFlags::Contains, 0);
	FOptions O;
	O.Seeds = 1; O.Frames = 600;
	O.OutFile = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("SoakLockstep"), TEXT("a120-fieldarrays.jsonl"));
	UJamCartridge* C = Load(*this, O.Cut);
	if (!C) return false;
	UJamSim* Probe = NewObject<UJamSim>();
	Probe->Init(C->Game, 1);
	const int32 Guest = GuestSlot(*Probe);
	FTiming Tm;
	const FMatch Clean = RunMatch(*this, O, *C, 1, Profile(TEXT("clean")), Tm);
	TestTrue(TEXT("the clean match has no differing field"), Clean.FirstDiffFrame < 0 && !Clean.bDesyncDriver && Clean.bFinalEqual);
	const FMatch M = RunMatch(*this, O, *C, 1, Profile(TEXT("inject-movehist7")), Tm);
	Emit(O, MatchJson(M));
	const FString Want = FString::Printf(TEXT("skater[%d].MoveHist[7] "), Guest);
	AddInfo(FString::Printf(TEXT("seeded MoveHist[7] of skater %d at frame 300: first diff frame %lld field '%s', lockstep hash from %lld, driver desync at %lld"), Guest, M.FirstDiffFrame,
		*M.FirstDiffField, M.FirstLockHashDiff, M.DriverDesyncFrame));
	TestEqual(TEXT("found at its exact frame"), M.FirstDiffFrame, static_cast<int64>(300));
	TestTrue(FString::Printf(TEXT("named as %s... (got '%s')"), *Want, *M.FirstDiffField), M.FirstDiffField.StartsWith(Want));
	TestEqual(TEXT("the lockstep hash differs from the same frame"), M.FirstLockHashDiff, static_cast<int64>(300));
	TestTrue(TEXT("the driver raised DESYNC at the checkpoint"), M.bDesyncDriver && M.DriverDesyncFrame == 300);
	return true;
}
