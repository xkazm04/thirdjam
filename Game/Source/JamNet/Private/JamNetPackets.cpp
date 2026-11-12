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
}

FJamNetQuantInput FJamNetQuantInput::From(const FJamInput& In)
{
	FJamNetQuantInput Q;
	Q.X = static_cast<int16>(FMath::RoundToInt(FMath::Clamp(In.Move.X, -1.0, 1.0) * 32767.0));
	Q.Y = static_cast<int16>(FMath::RoundToInt(FMath::Clamp(In.Move.Y, -1.0, 1.0) * 32767.0));
	Q.Flags = (In.bShoot ? 1 : 0) | (In.bCheck ? 2 : 0) | (In.bBrake ? 4 : 0);
	return Q;
}

FJamInput FJamNetQuantInput::ToInput() const
{
	FJamInput In;
	In.Move = FVector2D(static_cast<double>(X) / 32767.0, static_cast<double>(Y) / 32767.0);
	In.bShoot = (Flags & 1) != 0;
	In.bCheck = (Flags & 2) != 0;
	In.bBrake = (Flags & 4) != 0;
	return In;
}

void FJamNetInputPacket::Write(TArray<uint8>& Out) const
{
	Out.Reset();
	FMemoryWriter W(Out);
	WriteHeader(W, JamNet::EType::Input);
	int64 First = FirstFrame, HF = HashFrame, SF = SenderFrame;
	uint64 H = Hash;
	uint8 Count = static_cast<uint8>(FMath::Min(Inputs.Num(), 255));
	W << First << SF << HF << H << Count;
	for (int32 i = 0; i < Count; ++i)
	{
		int16 X = Inputs[i].X, Y = Inputs[i].Y;
		uint8 F = Inputs[i].Flags;
		W << X << Y << F;
	}
}

bool FJamNetInputPacket::Read(const TArray<uint8>& Bytes, FJamNetInputPacket& Out)
{
	FMemoryReader R(Bytes);
	if (!ReadHeader(R, JamNet::EType::Input)) return false;
	uint8 Count = 0;
	R << Out.FirstFrame << Out.SenderFrame << Out.HashFrame << Out.Hash << Count;
	if (R.IsError() || R.Tell() + Count * 5 > R.TotalSize()) return false;
	Out.Inputs.SetNum(Count);
	for (int32 i = 0; i < Count; ++i) R << Out.Inputs[i].X << Out.Inputs[i].Y << Out.Inputs[i].Flags;
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
