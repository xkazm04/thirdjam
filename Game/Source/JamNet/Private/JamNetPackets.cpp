#include "JamNetPackets.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"

DEFINE_LOG_CATEGORY(LogJamNet);

namespace
{
	void WriteHeader(FMemoryWriter& W, JamNet::EType Type)
	{
		uint32 Magic = JamNet::Magic;
		uint8 Version = JamNet::Version;
		uint8 T = static_cast<uint8>(Type);
		W << Magic << Version << T;
	}

	bool ReadHeader(FMemoryReader& R, JamNet::EType Expected)
	{
		if (R.TotalSize() < 6) return false;
		uint32 Magic = 0; uint8 Version = 0; uint8 T = 0;
		R << Magic << Version << T;
		return Magic == JamNet::Magic && Version == JamNet::Version && T == static_cast<uint8>(Expected);
	}

	void WriteString(FMemoryWriter& W, const FString& S)
	{
		FTCHARToUTF8 Utf8(*S);
		uint8 Len = static_cast<uint8>(FMath::Min(Utf8.Length(), 64));
		W << Len;
		W.Serialize(const_cast<ANSICHAR*>(Utf8.Get()), Len);
	}

	bool ReadString(FMemoryReader& R, FString& Out)
	{
		uint8 Len = 0;
		R << Len;
		if (R.IsError() || R.Tell() + Len > R.TotalSize()) return false;
		TArray<ANSICHAR> Buf;
		Buf.SetNumZeroed(Len + 1);
		R.Serialize(Buf.GetData(), Len);
		Out = FString(UTF8_TO_TCHAR(Buf.GetData()));
		return true;
	}

	const TCHAR* Crockford = TEXT("0123456789ABCDEFGHJKMNPQRSTVWXYZ");

	int32 CrockfordValue(TCHAR C)
	{
		C = FChar::ToUpper(C);
		if (C == 'I' || C == 'L') C = '1';
		if (C == 'O') C = '0';
		for (int32 i = 0; i < 32; ++i) if (Crockford[i] == C) return i;
		return -1;
	}

	int16 Q16(double V) { return static_cast<int16>(FMath::RoundToInt(FMath::Clamp(V, -1.0, 1.0) * 32767.0)); }
	double U16(int16 V) { return static_cast<double>(V) / 32767.0; }
}

FJamNetQuantInput FJamNetQuantInput::From(const FJamInput& In)
{
	FJamNetQuantInput Q;
	Q.X = Q16(In.Move.X);
	Q.Y = Q16(In.Move.Y);
	Q.StickX = Q16(In.Stick.X);
	Q.StickY = Q16(In.Stick.Y);
	Q.Flags = (In.bShoot ? Shoot : 0) | (In.bCheck ? Check : 0) | (In.bBrake ? Brake : 0) | (In.bShootHeld ? ShootHeld : 0)
		| (In.bPass ? Pass : 0) | (In.bPoke ? Poke : 0) | (In.bLift ? Lift : 0) | (In.bDeke ? Deke : 0) | (In.bSprint ? Sprint : 0) | (In.bSpin ? Spin : 0) | (In.bPassHeld ? PassHeld : 0)
		| (In.bAbility ? Ability : 0);   // lane A84
	return Q;
}

FJamInput FJamNetQuantInput::ToInput() const
{
	FJamInput In;
	In.Move = FVector2D(U16(X), U16(Y));
	In.Stick = FVector2D(U16(StickX), U16(StickY));
	In.bShoot = (Flags & Shoot) != 0;
	In.bCheck = (Flags & Check) != 0;
	In.bBrake = (Flags & Brake) != 0;
	In.bShootHeld = (Flags & ShootHeld) != 0;
	In.bPass = (Flags & Pass) != 0;
	In.bPoke = (Flags & Poke) != 0;
	In.bLift = (Flags & Lift) != 0;
	In.bSprint = (Flags & Sprint) != 0;
	In.bDeke = (Flags & Deke) != 0;   // lane A39
	In.bSpin = (Flags & Spin) != 0;   // lane A51
	In.bPassHeld = (Flags & PassHeld) != 0;   // lane A77
	In.bAbility = (Flags & Ability) != 0;   // lane A84 (R-0041 point 3)
	return In;
}

TArray<FName> FJamNetQuantInput::CoveredFields()
{
	return { FName(TEXT("Move")), FName(TEXT("bShoot")), FName(TEXT("bShootHeld")), FName(TEXT("bPass")), FName(TEXT("bCheck")), FName(TEXT("bBrake")),
		FName(TEXT("Stick")), FName(TEXT("bPoke")), FName(TEXT("bLift")), FName(TEXT("bDeke")), FName(TEXT("bSprint")), FName(TEXT("bSpin")), FName(TEXT("bPassHeld")), FName(TEXT("bAbility")) };
}

void FJamNetInputPacket::Write(TArray<uint8>& Out) const
{
	Out.Reset();
	FMemoryWriter W(Out);
	WriteHeader(W, JamNet::EType::Input);
	uint32 First = static_cast<uint32>(FirstFrame), SF = static_cast<uint32>(SenderFrame);
	int32 HF = static_cast<int32>(HashFrame);
	uint64 H = Hash;
	uint8 Count = static_cast<uint8>(FMath::Min(Inputs.Num(), 255));
	W << First << SF << HF << H << Count;
	for (int32 i = 0; i < Count; ++i)
	{
		int16 X = Inputs[i].X, Y = Inputs[i].Y, SX = Inputs[i].StickX, SY = Inputs[i].StickY;
		uint16 F = Inputs[i].Flags;
		W << X << Y << SX << SY << F;
	}
}

bool FJamNetInputPacket::Read(const TArray<uint8>& Bytes, FJamNetInputPacket& Out)
{
	FMemoryReader R(Bytes);
	if (!ReadHeader(R, JamNet::EType::Input)) return false;
	uint32 First = 0, SF = 0; int32 HF = 0; uint8 Count = 0;
	R << First << SF << HF << Out.Hash << Count;
	if (R.IsError() || R.Tell() + Count * FJamNetQuantInput::WireBytes > R.TotalSize()) return false;
	Out.FirstFrame = First;
	Out.SenderFrame = SF;
	Out.HashFrame = HF;
	Out.Inputs.SetNum(Count);
	for (int32 i = 0; i < Count; ++i) R << Out.Inputs[i].X << Out.Inputs[i].Y << Out.Inputs[i].StickX << Out.Inputs[i].StickY << Out.Inputs[i].Flags;
	return !R.IsError();
}

void FJamNetLobbyMessage::Write(TArray<uint8>& Out) const
{
	Out.Reset();
	FMemoryWriter W(Out);
	WriteHeader(W, Type);
	WriteString(W, Name);
	WriteString(W, Cut);
	uint64 CH = CartridgeHash, S = Seed;
	int32 GI = GuestIndex, F = Flags;
	W << CH << S << GI << F;
}

bool FJamNetLobbyMessage::Read(const TArray<uint8>& Bytes, FJamNetLobbyMessage& Out)
{
	Out.Type = JamNetPeekType(Bytes);
	if (Out.Type == JamNet::EType::None || Out.Type == JamNet::EType::Input) return false;
	FMemoryReader R(Bytes);
	if (!ReadHeader(R, Out.Type)) return false;
	if (!ReadString(R, Out.Name) || !ReadString(R, Out.Cut)) return false;
	R << Out.CartridgeHash << Out.Seed << Out.GuestIndex << Out.Flags;
	return !R.IsError();
}

JamNet::EType JamNetPeekType(const TArray<uint8>& Bytes)
{
	if (Bytes.Num() < 6) return JamNet::EType::None;
	uint32 Magic = 0;
	FMemory::Memcpy(&Magic, Bytes.GetData(), 4);
	if (Magic != JamNet::Magic || Bytes[4] != JamNet::Version) return JamNet::EType::None;
	const uint8 T = Bytes[5];
	return (T >= 1 && T <= 5) ? static_cast<JamNet::EType>(T) : JamNet::EType::None;
}

FString FJamInviteCode::Encode(uint32 Ipv4, int32 PortIndex)
{
	PortIndex = FMath::Clamp(PortIndex, 0, JamNet::PortSlots - 1);
	uint64 V = (static_cast<uint64>(Ipv4) << 8) | (static_cast<uint64>(PortIndex) << 4);
	uint32 Check = 0;
	for (int32 i = 1; i < 10; ++i) Check ^= static_cast<uint32>((V >> (4 * i)) & 0xF);
	V |= Check & 0xF;
	FString S;
	for (int32 i = 7; i >= 0; --i) S.AppendChar(Crockford[(V >> (5 * i)) & 31]);
	return S.Left(4) + TEXT("-") + S.Mid(4);
}

bool FJamInviteCode::Decode(const FString& Text, uint32& OutIpv4, int32& OutPort)
{
	FString T = Text.TrimStartAndEnd();
	// Address form for tests and power users: a.b.c.d:port
	int32 Colon = INDEX_NONE;
	if (T.FindChar(':', Colon))
	{
		if (!ParseIp(T.Left(Colon), OutIpv4)) return false;
		OutPort = FCString::Atoi(*T.Mid(Colon + 1));
		return OutPort > 0 && OutPort < 65536;
	}
	FString Clean;
	for (TCHAR C : T) if (C != '-' && C != ' ') Clean.AppendChar(C);
	if (Clean.Len() != 8) return false;
	uint64 V = 0;
	for (TCHAR C : Clean)
	{
		const int32 D = CrockfordValue(C);
		if (D < 0) return false;
		V = (V << 5) | static_cast<uint64>(D);
	}
	uint32 Check = 0;
	for (int32 i = 1; i < 10; ++i) Check ^= static_cast<uint32>((V >> (4 * i)) & 0xF);
	if ((Check & 0xF) != (V & 0xF)) return false;
	OutIpv4 = static_cast<uint32>(V >> 8);
	OutPort = JamNet::BasePort + static_cast<int32>((V >> 4) & 0xF);
	return true;
}

FString FJamInviteCode::FormatIp(uint32 Ipv4)
{
	return FString::Printf(TEXT("%u.%u.%u.%u"), (Ipv4 >> 24) & 255, (Ipv4 >> 16) & 255, (Ipv4 >> 8) & 255, Ipv4 & 255);
}

bool FJamInviteCode::ParseIp(const FString& Dotted, uint32& Out)
{
	TArray<FString> Parts;
	Dotted.TrimStartAndEnd().ParseIntoArray(Parts, TEXT("."), false);
	if (Parts.Num() != 4) return false;
	Out = 0;
	for (const FString& P : Parts)
	{
		if (P.IsEmpty() || !P.IsNumeric()) return false;
		const int32 N = FCString::Atoi(*P);
		if (N < 0 || N > 255) return false;
		Out = (Out << 8) | static_cast<uint32>(N);
	}
	return true;
}

uint64 JamNetFnv64(const TArray<uint8>& Bytes)
{
	uint64 H = 1469598103934665603ull;
	for (uint8 B : Bytes) { H ^= B; H *= 1099511628211ull; }
	return H;
}

// ---- the scripted human (lane A38) ----

FJamInput FJamNetScriptedHuman::Think(const UJamSim& Sim, int32 Me)
{
	FJamInput In;
	if (!Sim.GetSkaters().IsValidIndex(Me)) return In;
	const FJamSkaterState& S = Sim.GetSkaters()[Me];
	const FVector2D PuckPos = Sim.GetPuck().Pos;
	const FVector2D Goal(Sim.AttackGoalX(S.Team), 0.0);
	const FVector2D ToGoal = (Goal - PuckPos).GetSafeNormal();
	const FVector2D Behind = PuckPos - ToGoal * 0.7;
	const FVector2D D = Behind - S.Pos;
	In.Move = D.Size() > 0.3 ? D.GetSafeNormal() : ToGoal;
	const FVector2D ToPuck = PuckPos - S.Pos;
	const double Dist = ToPuck.Size();
	const double GoalDist = FVector2D::Distance(PuckPos, Goal);
	const bool bHave = Dist < Sim.CarrierReachM(S.Index) && Dist > 1e-6 && FVector2D::DotProduct(S.Forward(), ToPuck / Dist) > 0.6;
	const int64 Frame = Sim.GetFrame();
	// In the faceoff: press at the drop (the first clean press wins the draw).
	if (Sim.IsDrawOpen() && S.bCentre) { In.bPoke = true; return In; }
	if (Sim.GetPhase() != EJamSimPhase::Playing) { Charge = 0; ChaseUntil = -1; return In; }
	// The proof's foul: without the puck, run at the nearest opponent who does not carry it and check on contact range (R-0013: a check on a
	// skater without the puck is a foul when penalties are on). Once every 10 s at most, chased for up to 90 frames.
	const int32 Carrier = Sim.CarrierIndex();
	if (!bHave && Carrier != Me)
	{
		if (ChaseUntil >= 0 && Frame <= ChaseUntil && Sim.GetSkaters().IsValidIndex(ChaseTarget))
		{
			const FJamSkaterState& O = Sim.GetSkaters()[ChaseTarget];
			const FVector2D ToO = O.Pos - S.Pos;
			if (!O.IsDown() && Carrier != O.Index)
			{
				In.Move = ToO.GetSafeNormal();
				if (ToO.Size() < 1.6) { In.bCheck = true; LastCheck = Frame; ChaseUntil = -1; }
				return In;
			}
			ChaseUntil = -1;
		}
		else if (Frame - LastCheck > 600)
		{
			int32 Best = -1; double BestD = 6.0;
			for (const FJamSkaterState& O : Sim.GetSkaters())
			{
				if (O.Team == S.Team || O.IsDown() || O.Index == Carrier) continue;
				const double Dd = FVector2D::Distance(O.Pos, S.Pos);
				if (Dd < BestD) { BestD = Dd; Best = O.Index; }
			}
			if (Best >= 0) { ChaseTarget = Best; ChaseUntil = Frame + 90; LastCheck = Frame; In.Move = (Sim.GetSkaters()[Best].Pos - S.Pos).GetSafeNormal(); return In; }
		}
	}
	FVector2D Aim;
	if (bHave && GoalDist > 30.0 && Frame - LastPass > 600 && Sim.BestPass(Me, Aim) >= 0)
	{
		In.bPass = true;
		LastPass = Frame;
		Charge = 0;
		return In;
	}
	const int32 Max = Sim.GetConfig().Shot.ChargeFramesMax;
	if (bHave && FVector2D::DotProduct(S.Forward(), ToGoal) > 0.97 && GoalDist < 30.0)
	{
		if (Max <= 0) { In.bShoot = true; return In; }
		if (Charge < Max) { In.bShootHeld = true; ++Charge; return In; }
		Charge = 0;   // release: the charged shot fires this frame
		return In;
	}
	if (Charge > 0 && bHave) { In.bShootHeld = true; ++Charge; if (Charge >= Max + 10) Charge = 0; return In; }   // keep the charge while turning in
	Charge = 0;
	return In;
}
