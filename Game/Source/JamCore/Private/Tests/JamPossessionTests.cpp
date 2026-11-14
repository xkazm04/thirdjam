// Lane A208 (track F0, R-0067 point 2: "Puck does not stick with players, it moves through them without locking"; game.stick.pickup, docs/experiments/POSSESSION.md).
//   Jam.Core.Possession.Staged   the staged scenes (a loose puck at 2 / 6 / 12 / 20 m/s into a standing skater's feet, body and blade from the front, the side
//                                and behind; skating over a resting puck at walk, glide and sprint; a pass at his feet, a metre off his stick, at his skates; a
//                                shot into his body) with the rule (the defaults) and at its neutral value (pickup.arc_m 0, pickup.body false): one POSSTAGE row
//                                a scene and arm. The rule: every scene ends with him carrying or the puck bounced off him; nothing passes through.
//   Jam.Core.Possession.Sweep    1,440 arrivals into a standing body at 2..40 m/s from 24 directions (puck.max_speed 60): with the rule 0 pass-throughs.
//   Jam.Core.Possession.Lock     the carried puck against the blade over a sprint and a hard turn: within 15 cm 99 % of the frames, never behind his skates.
//   Jam.Core.Possession.Neutral  the neutral value replays the game before the lane: a seeded match at arc_m 0 / body false has the same hash whatever
//                                arc_deg and sting_mps say (the functions do not run), and the rule changes it.
//   Bar.Possession.Trace         (not in the gate) matches with the scripted human (env JAM_POSS "level=rookie;player=novice;seeds=20;from=1;arm=rule|neutral;rink=hockey.regular"):
//                                every loose puck that crosses a standing skater's pickup arc, picked up or not and why; the carried puck's distance from the blade.
#include "Misc/AutomationTest.h"
#include "JamCartridge.h"
#include "JamSim.h"
#include "JamBrain.h"
#include "JamSimAdapter.h"
#include "JamPlaytester.h"
#include "DifficultyResolve.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamPossessionTests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

	bool LoadGame(FAutomationTestBase& T, FJamGameConfig& Out, const TCHAR* Name = TEXT("hockey.regular"))
	{
		TArray<FString> Errors;
		UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(Name), Errors);
		for (const FString& E : Errors) T.AddError(E);
		if (!C) return false;
		Out = C->Game;
		return true;
	}

	void Neutral(FJamGameConfig& G) { G.Stick.PickupArcM = 0.0; G.Stick.bPickupBody = false; }

	TArray<FJamInput> NoInput(const UJamSim* Sim) { TArray<FJamInput> In; In.SetNum(Sim->GetSkaters().Num()); return In; }

	enum class EOut : uint8 { His, Bounce, Through, Untaken, Short, Other };
	const TCHAR* OutName(EOut O)
	{
		switch (O) { case EOut::His: return TEXT("his"); case EOut::Bounce: return TEXT("bounce"); case EOut::Through: return TEXT("THROUGH"); case EOut::Untaken: return TEXT("untaken"); case EOut::Short: return TEXT("short"); default: return TEXT("other"); }
	}

	struct FScene
	{
		FString Name;
		int32 Src = 1;               // 0 = nobody (a placed puck), 1 = an opponent touched it last, 2 = a team-mate (a pass)
		FVector2D VVel = FVector2D::ZeroVector;   // the skater's own motion (skating over a puck)
		FVector2D PuckAt;            // the puck's start, relative to the skater (he stands at the origin facing +X)
		FVector2D PuckVel;
	};

	struct FResult
	{
		EOut Out = EOut::Short;
		int32 Inside = 0;            // frames with the puck's centre inside his body circle
		int32 FirstHis = -1;         // the frame he became the carrier
		double Closest = 1e9;        // the path's nearest approach to his centre
		double EndSpeed = 0.0;
	};

	double SegDist(const FVector2D& A, const FVector2D& B, const FVector2D& P)
	{
		const FVector2D D = B - A;
		const double L2 = D.SizeSquared();
		const double T = L2 > 1e-12 ? FMath::Clamp(FVector2D::DotProduct(P - A, D) / L2, 0.0, 1.0) : 0.0;
		return FVector2D::Distance(A + D * T, P);
	}

	// One staged arrival on its own sim: the skater V (the human's figure) at the origin facing +X, everyone else parked far away; the source skater
	// touches the puck first (it is the last toucher), then the puck starts at V + PuckAt with PuckVel. 75 frames, no input (he stands or glides).
	FResult Play(const FJamGameConfig& G, const FScene& Sc, int32 Frames = 75)
	{
		UJamSim* Sim = NewObject<UJamSim>();
		Sim->Init(G, 7);
		Sim->SkipFaceoff();
		const int32 V = Sim->GetHumanIndex();
		const int32 Team = Sim->GetSkaters()[V].Team;
		int32 Src = -1;
		for (const FJamSkaterState& S : Sim->GetSkaters())
		{
			if (S.Index == V) continue;
			if (Src < 0 && ((Sc.Src == 1 && S.Team != Team) || (Sc.Src == 2 && S.Team == Team))) Src = S.Index;
			Sim->SetSkater(S.Index, FVector2D(-35.0 + S.Index * 2.5, 18.0), FVector2D::ZeroVector, 0);
		}
		TArray<FJamInput> In = NoInput(Sim);
		if (Src >= 0)
		{
			const FJamSkaterState& S = Sim->GetSkaters()[Src];
			Sim->SetPuck(S.Pos + S.Forward() * 0.45, FVector2D::ZeroVector);
			Sim->Step(In);
		}
		Sim->SetSkater(V, FVector2D::ZeroVector, Sc.VVel, 0);
		Sim->SetPuck(FVector2D(20.0, -14.0), FVector2D::ZeroVector);   // away from everybody while his blade settles
		Sim->Step(In);
		Sim->SetSkater(V, FVector2D::ZeroVector, Sc.VVel, 0);
		Sim->Step(In);   // one more frame for the blade at the origin
		const FVector2D Origin = Sim->GetSkaters()[V].Pos;
		Sim->SetPuck(Origin + Sc.PuckAt, Sc.PuckVel);
		const double R = Sim->GetConfig().Puck.RadiusM;
		FResult Res;
		bool bTouched = false;
		int32 HisRun = 0;
		for (int32 F = 0; F < Frames; ++F)
		{
			const FVector2D From = Sim->GetPuck().Pos;
			{   // he looks at the puck: the blade aimed its way at rest length (as a person's mouse, as a bot's blade near a puck)
				const FVector2D ToP = From - Sim->GetSkaters()[V].Pos;
				In[V].Stick = (ToP.Size() < 4.0 && ToP.Size() > 1e-3) ? ToP.GetSafeNormal() * 0.5 : FVector2D::ZeroVector;
			}
			Sim->Step(In);
			const FJamSkaterState& S = Sim->GetSkaters()[V];
			const FVector2D P = Sim->GetPuck().Pos;
			Res.Closest = FMath::Min(Res.Closest, SegDist(From, P, S.Pos));
			if (FVector2D::Distance(P, S.Pos) < UJamSim::SkaterRadius + R - 0.02) ++Res.Inside;
			if (Sim->GetPuck().LastTouch == V) bTouched = true;
			if (Sim->CarrierIndex() == V) { if (Res.FirstHis < 0) Res.FirstHis = F + 1; ++HisRun; } else HisRun = 0;
			if (Sim->GetPuck().LastTouch != V && Sim->GetPuck().LastTouch != Src && Sim->GetPuck().LastTouch >= 0) { Res.Out = EOut::Other; return Res; }
		}
		Res.EndSpeed = (Sim->GetPuck().Vel - Sim->GetSkaters()[V].Vel).Size();
		const bool bHisNow = Sim->CarrierIndex() == V && HisRun >= 10;
		if (bHisNow) Res.Out = EOut::His;
		else if (bTouched && Res.Inside <= 1) Res.Out = Sim->CarrierIndex() == V ? EOut::Untaken : EOut::Bounce;
		else if (!bTouched && Res.Closest < UJamSim::SkaterRadius + R) Res.Out = EOut::Through;
		else if (bTouched) Res.Out = EOut::Through;   // touched but sat inside him for frames: through, not stopped
		else if (FVector2D::Distance(Sim->GetPuck().Pos, Sim->GetSkaters()[V].Pos) <= Sim->CarrierReachM(V)) Res.Out = EOut::Untaken;
		else Res.Out = Res.Closest < 1.7 ? EOut::Untaken : EOut::Short;
		return Res;
	}

	TArray<FScene> StagedScenes(const FJamGameConfig& G)
	{
		TArray<FScene> Out;
		const double BladeX = UJamSim::SkaterRadius + 0.5 * G.Stick.ReachM;   // the rest blade (no aim): half reach ahead
		struct FFrom { const TCHAR* Name; FVector2D Dir; };
		const FFrom Froms[] = { { TEXT("front"), FVector2D(1, 0) }, { TEXT("side"), FVector2D(0, 1) }, { TEXT("behind"), FVector2D(-1, 0) } };
		struct FAt { const TCHAR* Name; FVector2D P; };
		const FAt Ats[] = { { TEXT("feet"), FVector2D(0, 0) }, { TEXT("body"), FVector2D(0, 0.3) }, { TEXT("blade"), FVector2D(BladeX, 0) } };
		for (double Sp : { 2.0, 6.0, 12.0, 20.0 })
			for (const FFrom& Fr : Froms)
				for (const FAt& At : Ats)
				{
					FScene S;
					S.Name = FString::Printf(TEXT("loose %2.0f m/s %-6s %-5s"), Sp, Fr.Name, At.Name);
					S.Src = 1;
					const double Start = Sp <= 2.0 ? 1.6 : 3.0;
					S.PuckAt = At.P + Fr.Dir * Start;
					S.PuckVel = -Fr.Dir * Sp;
					Out.Add(S);
				}
		const TCHAR* Gaits[] = { TEXT("walk"), TEXT("glide"), TEXT("sprint") };
		const double Speeds[] = { 3.0, 6.0, 9.0 };
		for (int32 Gi = 0; Gi < 3; ++Gi)
			for (double Lat : { 0.0, 0.3, 0.8 })
			{
				FScene S;
				S.Name = FString::Printf(TEXT("skate over %-6s lateral %.1f"), Gaits[Gi], Lat);
				S.Src = 1;
				S.VVel = FVector2D(Speeds[Gi], 0);
				S.PuckAt = FVector2D(3.0, Lat);
				S.PuckVel = FVector2D::ZeroVector;
				Out.Add(S);
			}
		for (double Sp : { 8.0, 14.0, 20.0, 26.0 })   // a mate's puck over sting_mps x 1.5 (27 m/s) is a shot through him: it rings off
		{
			struct FPass { const TCHAR* Name; FVector2D At; };
			const FPass Ps[] = { { TEXT("at his feet"), FVector2D(0, 0) }, { TEXT("1 m off his stick"), FVector2D(BladeX, 1.0) }, { TEXT("at his skates"), FVector2D(-0.3, 0.2) } };
			for (const FPass& P : Ps)
			{
				FScene S;
				S.Name = FString::Printf(TEXT("pass %2.0f m/s %s"), Sp, P.Name);
				S.Src = 2;
				const FVector2D Dir = FVector2D(1, 1).GetSafeNormal();   // from his front left, as a mate's pass comes
				S.PuckAt = P.At + Dir * 4.0;
				S.PuckVel = -Dir * Sp;
				Out.Add(S);
			}
		}
		for (double Sp : { 25.0, 35.0 })
		{
			FScene S;
			S.Name = FString::Printf(TEXT("shot %2.0f m/s into his body"), Sp);
			S.Src = 1;
			S.PuckAt = FVector2D(-5.0, 0.1);
			S.PuckVel = FVector2D(Sp, 0);
			Out.Add(S);
		}
		return Out;
	}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPossessionStaged, "Jam.Core.Possession.Staged", JamPossessionTests::Flags)
bool FJamPossessionStaged::RunTest(const FString& Parameters)
{
	using namespace JamPossessionTests;
	FJamGameConfig G;
	if (!LoadGame(*this, G)) return false;
	G.Puck.MaxSpeed = 60.0;   // the doubled shot of tonight (lane A207); the defaults stay otherwise
	TestTrue(TEXT("the rule ships on: pickup.arc_m 1.5, pickup.body true"), G.Stick.PickupArcM > 1.4 && G.Stick.bPickupBody);
	FJamGameConfig N = G;
	Neutral(N);
	const TArray<FScene> Scenes = StagedScenes(G);
	int32 Bad = 0, ThroughN = 0, ThroughR = 0, HisN = 0, HisR = 0;
	for (const FScene& Sc : Scenes)
	{
		const FResult A = Play(N, Sc);
		const FResult B = Play(G, Sc);
		if (A.Out == EOut::Through) ++ThroughN;
		if (B.Out == EOut::Through) ++ThroughR;
		if (A.Out == EOut::His) ++HisN;
		if (B.Out == EOut::His) ++HisR;
		AddInfo(FString::Printf(TEXT("POSSTAGE %-36s | neutral %-7s inside %2d his@%3d | rule %-7s inside %2d his@%3d"), *Sc.Name, OutName(A.Out), A.Inside, A.FirstHis, OutName(B.Out), B.Inside, B.FirstHis));
		const bool bSting = Sc.Src == 1 && Sc.PuckVel.Size() > G.Stick.PickupStingMps;
		const bool bOk = B.Out == EOut::His || (bSting && B.Out == EOut::Bounce);
		if (!bOk) { ++Bad; AddError(FString::Printf(TEXT("the rule: %s ends %s (want his%s)"), *Sc.Name, OutName(B.Out), bSting ? TEXT(" or a bounce") : TEXT(""))); }
		if (B.Inside > 8) { ++Bad; AddError(FString::Printf(TEXT("the rule: %s: the puck sat inside his body %d frames (want 8 at most: the carry draws it through to the blade)"), *Sc.Name, B.Inside)); }
	}
	AddInfo(FString::Printf(TEXT("POSSTAGE total %d scenes | neutral: his %d, through %d | rule: his %d, through %d"), Scenes.Num(), HisN, ThroughN, HisR, ThroughR));
	TestEqual(TEXT("the rule: nothing passes through him"), ThroughR, 0);
	// An opponent carries his puck into the standing skater at 4, 7 and 9 m/s: with the rule the puck stops at the body (never inside it) and stays the carrier's.
	for (double Sp : { 4.0, 7.0, 9.0 })
	{
		for (int32 Arm = 0; Arm < 2; ++Arm)
		{
			UJamSim* Sim = NewObject<UJamSim>();
			Sim->Init(Arm == 0 ? N : G, 7);
			Sim->SkipFaceoff();
			const int32 V = Sim->GetHumanIndex();
			int32 O = -1;
			for (const FJamSkaterState& S : Sim->GetSkaters()) { if (O < 0 && S.Team != Sim->GetSkaters()[V].Team) O = S.Index; if (S.Index != V) Sim->SetSkater(S.Index, FVector2D(-35.0 + S.Index * 2.5, 18.0), FVector2D::ZeroVector, 0); }
			Sim->SetSkater(V, FVector2D(0, 0), FVector2D::ZeroVector, UE_DOUBLE_PI);
			Sim->SetSkater(O, FVector2D(-4.0, -0.9), FVector2D(Sp, 0), 0);   // his body passes 0.9 m beside him (no body contact); the blade, aimed forward-left, passes through him
			TArray<FJamInput> In = NoInput(Sim);
			Sim->SetPuck(FVector2D(-3.2, -0.3), FVector2D(Sp, 0));
			Sim->Step(In);
			int32 Inside = 0, Kept = 0, VTook = 0;
			const double R = Sim->GetConfig().Puck.RadiusM;
			for (int32 F = 0; F < 40; ++F)
			{
				In[O].Move = FVector2D(1, 0); In[O].Stick = FVector2D(0.7, 0.7);
				Sim->Step(In);
				if (FVector2D::Distance(Sim->GetPuck().Pos, Sim->GetSkaters()[V].Pos) < UJamSim::SkaterRadius + R - 0.02) ++Inside;
				if (Sim->CarrierIndex() == O) ++Kept;
				if (Sim->GetPuck().LastTouch == V) ++VTook;
			}
			// No assertion on "his touch took it": the puck held at his body while the carrier skates on leaves the carrier's reach and is then a loose puck (his).
			AddInfo(FString::Printf(TEXT("POSSTAGE carried into him at %.0f m/s | %s: puck inside his body %d of 40 frames, the carrier keeps it %d, his touch took it %d"), Sp, Arm == 0 ? TEXT("neutral") : TEXT("rule"), Inside, Kept, VTook));
			if (Arm == 1)
			{
				TestTrue(*FString::Printf(TEXT("the rule: a carried puck at %.0f m/s never passes through a body (%d frames inside)"), Sp, Inside), Inside <= 1);
			}
		}
	}
	TestEqual(TEXT("the rule: every scene ends his or bounced"), Bad, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPossessionSweep, "Jam.Core.Possession.Sweep", JamPossessionTests::Flags)
bool FJamPossessionSweep::RunTest(const FString& Parameters)
{
	using namespace JamPossessionTests;
	FJamGameConfig G;
	if (!LoadGame(*this, G)) return false;
	G.Puck.MaxSpeed = 60.0;
	FJamGameConfig N = G;
	Neutral(N);
	int32 Arrivals = 0, ThroughR = 0, ThroughN = 0, HisR = 0, BounceR = 0, HisN = 0;
	for (int32 Sp = 2; Sp <= 40; Sp += 2)
		for (int32 Deg = 0; Deg < 360; Deg += 15)
			for (double Lat : { -0.35, 0.0, 0.35 })
			{
				const double A = FMath::DegreesToRadians(static_cast<double>(Deg));
				const FVector2D Dir(FMath::Cos(A), FMath::Sin(A));
				FScene Sc;
				Sc.Src = (Deg / 15) % 3 == 0 ? 2 : 1;
				Sc.PuckAt = FVector2D(-Dir.Y, Dir.X) * Lat + Dir * (Sp <= 4 ? 1.8 : 3.5);
				Sc.PuckVel = -Dir * static_cast<double>(Sp);
				const FResult R = Play(G, Sc, 45);
				const FResult Rn = Play(N, Sc, 45);
				++Arrivals;
				if (R.Out == EOut::Through) { ++ThroughR; if (ThroughR <= 10) AddError(FString::Printf(TEXT("the rule: %d m/s from %d deg lateral %.2f passes through (inside %d frames)"), Sp, Deg, Lat, R.Inside)); }
				if (Rn.Out == EOut::Through) ++ThroughN;
				if (R.Out == EOut::His) ++HisR;
				if (R.Out == EOut::Bounce) ++BounceR;
				if (Rn.Out == EOut::His) ++HisN;
			}
	AddInfo(FString::Printf(TEXT("POSSWEEP %d arrivals at 2..40 m/s | rule: his %d, bounced %d, through %d | neutral: his %d, through %d"), Arrivals, HisR, BounceR, ThroughR, HisN, ThroughN));
	TestTrue(TEXT("at least 1,000 arrivals"), Arrivals >= 1000);
	TestEqual(TEXT("the rule: 0 pass-throughs"), ThroughR, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPossessionLock, "Jam.Core.Possession.Lock", JamPossessionTests::Flags)
bool FJamPossessionLock::RunTest(const FString& Parameters)
{
	using namespace JamPossessionTests;
	FJamGameConfig G;
	if (!LoadGame(*this, G)) return false;
	UJamSim* Sim = NewObject<UJamSim>();
	Sim->Init(G, 3);
	Sim->SkipFaceoff();
	const int32 V = Sim->GetHumanIndex();
	for (const FJamSkaterState& S : Sim->GetSkaters()) if (S.Index != V) Sim->SetSkater(S.Index, FVector2D(-35.0 + S.Index * 2.5, 18.0), FVector2D::ZeroVector, 0);
	Sim->SetSkater(V, FVector2D(-20.0, 0.0), FVector2D::ZeroVector, 0);
	TArray<FJamInput> In = NoInput(Sim);
	Sim->SetPuck(FVector2D(-19.4, 0.0), FVector2D::ZeroVector);
	Sim->Step(In);
	int32 Frames = 0, Near = 0, Behind = 0, Lost = 0;
	double Worst = 0.0;
	for (int32 F = 0; F < 240; ++F)
	{
		FJamInput& I = In[V];
		I.Stick = FVector2D::ZeroVector;
		if (F < 90) { I.Move = FVector2D(1, 0); I.bSprint = true; }                                     // a sprint up the ice
		else if (F < 150) { I.Move = FVector2D(0, F < 120 ? 1 : -1); I.bSprint = false; }               // a hard turn left, then right
		else { I.Move = FVector2D(-1, 0); I.bSprint = F < 200; }                                         // back the other way
		Sim->Step(In);
		const FJamSkaterState& S = Sim->GetSkaters()[V];
		if (Sim->CarrierIndex() != V) { ++Lost; continue; }
		if (F < 6) continue;   // the first touch settles
		++Frames;
		const double D = FVector2D::Distance(Sim->GetPuck().Pos, S.BladePos);
		Worst = FMath::Max(Worst, D);
		if (D <= 0.15) ++Near;
		if (FVector2D::DotProduct(Sim->GetPuck().Pos - S.Pos, S.Forward()) < -UJamSim::SkaterRadius) ++Behind;
	}
	AddInfo(FString::Printf(TEXT("POSLOCK carry frames %d, within 15 cm of the blade %d (%.1f %%), worst %.3f m, behind his skates %d, frames not his %d"), Frames, Near, Frames ? 100.0 * Near / Frames : 0.0, Worst, Behind, Lost));
	TestEqual(TEXT("he keeps it through the sprint and the turns"), Lost, 0);
	TestTrue(TEXT("within 15 cm of the blade 99 % of the carry frames"), Frames > 0 && Near >= 0.99 * Frames);
	TestEqual(TEXT("never behind his skates"), Behind, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPossessionNeutral, "Jam.Core.Possession.Neutral", JamPossessionTests::Flags)
bool FJamPossessionNeutral::RunTest(const FString& Parameters)
{
	using namespace JamPossessionTests;
	TArray<FString> Errors;
	UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")), Errors);
	if (!C) return false;
	auto Hash = [&](TFunctionRef<void(FJamGameConfig&)> Edit)
	{
		UJamCartridge* M = DuplicateObject<UJamCartridge>(C, GetTransientPackage());
		Edit(M->Game);
		JamAI::FJamSimAdapter Sim(*M);
		const JamAI::FJamMatchConfig Cfg = JamAI::ToMatchConfigResolved(*M, TEXT("pro"));
		return JamAI::RunPlaytestMatch(Sim, Cfg, 2, JamAI::EPlaytester::Regular, 12000, M->Game.Shot.ChargeFramesMax).Hash;
	};
	const uint64 H0 = Hash([](FJamGameConfig& G) { Neutral(G); });
	const uint64 H1 = Hash([](FJamGameConfig& G) { Neutral(G); G.Stick.PickupArcDeg = 170.0; G.Stick.PickupStingMps = 3.0; });
	const uint64 H2 = Hash([](FJamGameConfig& G) {});
	AddInfo(FString::Printf(TEXT("POSNEUTRAL neutral %016llx, neutral with the other two numbers moved %016llx, the rule %016llx"), (unsigned long long)H0, (unsigned long long)H1, (unsigned long long)H2));
	TestEqual(TEXT("at arc_m 0 and body false nothing of the rule runs"), H1, H0);
	TestNotEqual(TEXT("the rule changes the game"), H2, H0);
	return true;
}

	// ---- Bar.Possession.Trace: the match counts ----
	struct FTrace
	{
		JamAI::FJamSimAdapter* Sim = nullptr;
		int64 LastFrame = -1;
		double ArcM = 1.5, ArcDeg = 60.0;
		// open episodes: a loose puck crossed skater i's arc at frame Open[i] (-1 = none)
		TArray<int64> Open;
		TArray<int32> OpenWhy;
		bool bPrevLoose = false;
		int64 Episodes = 0, Taken = 0, ByOther = 0, NotTaken = 0;
		int64 Why[12] = {};
		int64 CarryFrames = 0, CarryNear = 0, CarryBehind = 0, CarryPlain = 0, CarryPlainNear = 0;
		double CarryWorst = 0.0;
		int64 Through = 0;   // frames a loose puck's centre sat inside a standing body
		TArray<int32> CarrierRun;
		int32 PrevTouch = -1;
		// Opponents' long shots (20-35 m from the net they attack): blocked by a body or stick of the human's side (not the crease man), saved by the crease man, a goal, or other.
		int64 ShotOpen = -1; int32 ShotBy = -1; int64 LongShots = 0, LongBlocked = 0, LongCrease = 0, LongGoal = 0, LongOther = 0;
		TMap<FString, int64> Hands;   // hand changes: "<taker side> <taker role> from <source>"
	};
	const TCHAR* WhyName(int32 W)
	{
		static const TCHAR* N[] = { TEXT("slow-at-blade(<6m/s)"), TEXT("fast-unaimed-opp[design]"), TEXT("own-puck"), TEXT("behind-arc(body side)"), TEXT("in-air"), TEXT("other"), TEXT("mate-pass-wide(>1.2 m of the blade)[design]"), TEXT("fast(sting)[design]"), TEXT("opp-in-flight-off-the-blade[design]"), TEXT("just-stripped-cooldown[design]"), TEXT("stumbling-carrier-keeps[design]"), TEXT("mate-pass-near") };
		return N[FMath::Clamp(W, 0, 11)];
	}

	void Trace(void* Ctx, const JamAI::FJamAIWorld&, const JamAI::FJamAIInput*, const JamAI::FJamBrain&, int64_t, bool)
	{
		FTrace& T = *static_cast<FTrace*>(Ctx);
		const UJamSim* Sim = T.Sim->GetSim();
		if (!Sim || Sim->GetFrame() == T.LastFrame) return;
		T.LastFrame = Sim->GetFrame();
		const TArray<FJamSkaterState>& Sk = Sim->GetSkaters();
		if (T.Open.Num() != Sk.Num()) { T.Open.Init(-1, Sk.Num()); T.OpenWhy.Init(5, Sk.Num()); T.CarrierRun.Init(0, Sk.Num()); }
		const FJamPuckState& P = Sim->GetPuck();
		const bool bPlay = Sim->GetPhase() == EJamSimPhase::Playing && !Sim->IsDrawOpen();
		const int32 Ci = Sim->CarrierIndex();
		const bool bRiding = Ci >= 0 && ((P.Vel - Sk[Ci].Vel).Size() < 4.0 || Sk[Ci].SettleFrames > 0);
		const int64 Now = Sim->GetFrame();
		if (P.LastTouch != T.PrevTouch && Sk.IsValidIndex(P.LastTouch) && bPlay)
		{
			int32 Me = -1;
			for (const FJamSkaterState& S : Sk) if (S.bHuman) Me = S.Index;
			const FJamSkaterState& Y = Sk[P.LastTouch];
			const bool bHumanSide = Me >= 0 && Y.Team == Sk[Me].Team;
			const FString Src = !Sk.IsValidIndex(T.PrevTouch) ? TEXT("none") : (Sk[T.PrevTouch].Team == Y.Team ? TEXT("mate") : TEXT("opp"));
			const FString Key = FString::Printf(TEXT("%s %s from %s%s"), bHumanSide ? TEXT("us") : TEXT("them"), Y.bHuman ? TEXT("HUMAN") : *Y.Role.ToString(), *Src, P.Vel.Size() > 12.0 ? TEXT(" fast") : TEXT(""));
			T.Hands.FindOrAdd(Key) += 1;
		}
		T.PrevTouch = P.LastTouch;
		{
			int32 Me = -1;
			for (const FJamSkaterState& S : Sk) if (S.bHuman) Me = S.Index;
			if (T.ShotOpen >= 0)
			{
				if (Sim->GetPhase() == EJamSimPhase::GoalPause) { ++T.LongGoal; T.ShotOpen = -1; }
				else if (P.LastTouch != T.ShotBy && Sk.IsValidIndex(P.LastTouch))
				{
					const FJamSkaterState& Y = Sk[P.LastTouch];
					if (Me >= 0 && Y.Team == Sk[Me].Team) { if (Sim->InCreaseStance(Y)) ++T.LongCrease; else ++T.LongBlocked; } else ++T.LongOther;
					T.ShotOpen = -1;
				}
				else if (Now - T.ShotOpen > 150 || !bPlay) { ++T.LongOther; T.ShotOpen = -1; }
			}
			if (T.ShotOpen < 0 && Me >= 0 && bPlay)
				for (const FJamSkaterState& S : Sk)
					if (S.bShotThisFrame && S.Team != Sk[Me].Team)
					{
						const double Dn = FVector2D::Distance(S.Pos, FVector2D(Sim->AttackGoalX(S.Team), 0.0));
						if (Dn >= 20.0 && Dn <= 35.0) { T.ShotOpen = Now; T.ShotBy = S.Index; ++T.LongShots; }
					}
		}
		// Close episodes: 12 frames after the crossing, or as soon as somebody carries.
		for (int32 i = 0; i < Sk.Num(); ++i)
		{
			if (T.Open[i] < 0) continue;
			const bool bCarried = bRiding;
			if (bCarried || Now - T.Open[i] >= 12 || !bPlay)
			{
				if (bCarried && Ci == i) ++T.Taken;
				else if (bCarried) ++T.ByOther;
				else { ++T.NotTaken; ++T.Why[T.OpenWhy[i]]; }
				T.Open[i] = -1;
			}
		}
		// The carry.
		if (bPlay && bRiding)
		{
			const FJamSkaterState& C = Sk[Ci];
			if (++T.CarrierRun[Ci] > 6)
			{
				++T.CarryFrames;
				const double D = FVector2D::Distance(P.Pos, C.BladePos);
				T.CarryWorst = FMath::Max(T.CarryWorst, D);
				if (D <= 0.15) ++T.CarryNear;
				if (C.SpinFrames == 0 && C.DekeFrames == 0) { ++T.CarryPlain; if (D <= 0.15) ++T.CarryPlainNear; }
				if (FVector2D::DotProduct(P.Pos - C.Pos, C.Forward()) < -UJamSim::SkaterRadius) ++T.CarryBehind;
			}
		}
		for (int32 i = 0; i < Sk.Num(); ++i) if (!(bRiding && Ci == i)) T.CarrierRun[i] = 0;
		// New crossings: the frame's path against every standing skater's arc, the puck loose at the frame's start.
		const bool bLooseNow = bPlay && !bRiding;
		if (bPlay && (T.bPrevLoose || bLooseNow) && P.Z < UJamSim::BladeClearM)
		{
			const double R = Sim->GetConfig().Puck.RadiusM;
			const FVector2D A = P.PrevPos, B = P.Pos;
			for (const FJamSkaterState& S : Sk)
			{
				if (!S.CanAct() || S.bCrossBraced || T.Open[S.Index] >= 0) continue;
				const FVector2D D = B - A;
				const double L2 = D.SizeSquared();
				const double U = L2 > 1e-12 ? FMath::Clamp(FVector2D::DotProduct(S.Pos - A, D) / L2, 0.0, 1.0) : 1.0;
				const FVector2D At = A + D * U;
				const FVector2D To = At - S.Pos;
				const double Dist = To.Size();
				if (Dist < UJamSim::SkaterRadius + R - 0.02 && bLooseNow) ++T.Through;
				const double CosA = FMath::Cos(FMath::DegreesToRadians(T.ArcDeg));
				const FVector2D Bd = (S.BladePos - S.Pos).GetSafeNormal();
				const bool bArc = Dist <= T.ArcM && (Dist < 1e-6 || FVector2D::DotProduct(To, S.Forward()) >= CosA * Dist || FVector2D::DotProduct(To, Bd) >= CosA * Dist);
				const bool bBody = Dist <= UJamSim::SkaterRadius + R;
				const bool bBlade = FVector2D::Distance(At, S.BladePos) <= 0.6;
				if (!(bArc || bBody || bBlade)) continue;
				if (P.LastTouch == S.Index && FVector2D::DotProduct(P.Vel - S.Vel, To) >= 0.0) continue;   // his own puck leaving him: not a crossing
				if (Sim->CarrierIndex() == S.Index && bRiding) continue;
				++T.Episodes;
				T.Open[S.Index] = Now;
				const double Rel = (P.Vel - S.Vel).Size();
				const bool bOpp = Sk.IsValidIndex(P.LastTouch) && Sk[P.LastTouch].Team != S.Team;
				const bool bMate = Sk.IsValidIndex(P.LastTouch) && Sk[P.LastTouch].Team == S.Team && P.LastTouch != S.Index;
				int32 W = 5;
				const bool bBlade12 = FVector2D::Distance(At, S.BladePos) <= 1.2;
				bool bStumbleKeep = false;
				if (Sk.IsValidIndex(P.LastTouch) && P.LastTouch != S.Index) { const FJamSkaterState& L = Sk[P.LastTouch]; bStumbleKeep = L.IsStumbling() && !L.IsDown() && FVector2D::Distance(L.Pos, P.Pos) <= Sim->CarrierReachM(L.Index); }
				if (P.LastTouch == S.Index) W = 2;
				else if (S.DeflectCooldown > 0) W = 9;
				else if (bStumbleKeep) W = 10;
				else if (bOpp && Rel > 18.0) W = 7;
				else if (bOpp && Rel > 6.0 && !bBody && !(bBlade && S.bStickAimed)) W = 8;
				else if (bMate && Rel > 6.0 && !bBody && !bBlade12) W = 6;
				else if (bMate) W = 11;
				else if (!bBody && !bBlade && !bArc) W = 3;
				else if (!bBody && Rel < 6.0) W = 0;
				else if (!bBody && bOpp && !S.bStickAimed) W = 1;
				T.OpenWhy[S.Index] = W;
			}
		}
		T.bPrevLoose = bLooseNow;
	}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBarPossessionTrace, "Bar.Possession.Trace", JamPossessionTests::Flags)
bool FJamBarPossessionTrace::RunTest(const FString& Parameters)
{
	using namespace JamPossessionTests;
	FString Level = TEXT("rookie"), Player = TEXT("novice"), Arm = TEXT("rule"), Rink = TEXT("hockey.regular");
	int32 Seeds = 20, From = 1;
	{
		TArray<FString> Parts;
		FPlatformMisc::GetEnvironmentVariable(TEXT("JAM_POSS")).ParseIntoArray(Parts, TEXT(";"));
		for (const FString& Part : Parts)
		{
			FString K, V;
			if (!Part.Split(TEXT("="), &K, &V)) continue;
			if (K == TEXT("level")) Level = V; else if (K == TEXT("player")) Player = V; else if (K == TEXT("arm")) Arm = V; else if (K == TEXT("rink")) Rink = V;
			else if (K == TEXT("seeds")) Seeds = FCString::Atoi(*V); else if (K == TEXT("from")) From = FCString::Atoi(*V);
		}
	}
	TArray<FString> Errors;
	UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(*Rink), Errors);
	for (const FString& E : Errors) AddError(E);
	if (!C) return false;
	const JamAI::EPlaytester Kind = Player == TEXT("novice") ? JamAI::EPlaytester::Novice : (Player == TEXT("good") ? JamAI::EPlaytester::Good : JamAI::EPlaytester::Regular);
	FTrace Sum;
	int32 Wins = 0;
	for (int32 Seed = From; Seed < From + Seeds; ++Seed)
	{
		UJamCartridge* M = DuplicateObject<UJamCartridge>(C, GetTransientPackage());
		if (Arm == TEXT("neutral")) Neutral(M->Game);
		else if (Arm == TEXT("bodyonly")) M->Game.Stick.PickupArcM = 0.0;
		else if (Arm == TEXT("arconly")) M->Game.Stick.bPickupBody = false;
		JamAI::FJamSimAdapter Sim(*M);
		FTrace T;
		T.Sim = &Sim;
		const JamAI::FJamMatchConfig Cfg = JamAI::ToMatchConfigResolved(*M, *Level);
		const JamAI::FJamPlaytestStats S = JamAI::RunPlaytestMatch(Sim, Cfg, static_cast<uint64>(Seed), Kind, 36000, M->Game.Shot.ChargeFramesMax, false, &Trace, &T, 1);
		if (S.GoalsFor > S.GoalsAgainst) ++Wins;
		AddInfo(FString::Printf(TEXT("POSTRACE match arm=%s level=%s seed=%d %d:%d hash %016llx crossings %lld taken %lld other %lld not %lld"), *Arm, *Level, Seed, S.GoalsFor, S.GoalsAgainst,
			(unsigned long long)S.Hash, (long long)T.Episodes, (long long)T.Taken, (long long)T.ByOther, (long long)T.NotTaken));
		Sum.Episodes += T.Episodes; Sum.Taken += T.Taken; Sum.ByOther += T.ByOther; Sum.NotTaken += T.NotTaken; Sum.Through += T.Through;
		for (int32 w = 0; w < 12; ++w) Sum.Why[w] += T.Why[w];
		for (const TPair<FString, int64>& H : T.Hands) Sum.Hands.FindOrAdd(H.Key) += H.Value;
		Sum.LongShots += T.LongShots; Sum.LongBlocked += T.LongBlocked; Sum.LongCrease += T.LongCrease; Sum.LongGoal += T.LongGoal; Sum.LongOther += T.LongOther;
		Sum.CarryFrames += T.CarryFrames; Sum.CarryNear += T.CarryNear; Sum.CarryPlain += T.CarryPlain; Sum.CarryPlainNear += T.CarryPlainNear; Sum.CarryBehind += T.CarryBehind; Sum.CarryWorst = FMath::Max(Sum.CarryWorst, T.CarryWorst);
	}
	const int64 Closed = Sum.Taken + Sum.ByOther + Sum.NotTaken;
	AddInfo(FString::Printf(TEXT("POSTRACE sum arm=%s level=%s player=%s seeds=%d wins=%d | crossings of a standing skater's arc by a loose puck %lld: his %lld (%.1f %%), another's %lld, nobody's %lld (%.1f %%) | frames a loose puck sat inside a standing body %lld | carry frames %lld: within 15 cm of the blade %.2f %%, behind his skates %lld, worst %.2f m"),
		*Arm, *Level, *Player, Seeds, Wins, (long long)Closed, (long long)Sum.Taken, Closed ? 100.0 * Sum.Taken / Closed : 0.0, (long long)Sum.ByOther, (long long)Sum.NotTaken, Closed ? 100.0 * Sum.NotTaken / Closed : 0.0,
		(long long)Sum.Through, (long long)Sum.CarryFrames, Sum.CarryFrames ? 100.0 * Sum.CarryNear / Sum.CarryFrames : 0.0, (long long)Sum.CarryBehind, Sum.CarryWorst));
	{
		const int64 Design = Sum.Why[1] + Sum.Why[6] + Sum.Why[7] + Sum.Why[8] + Sum.Why[9] + Sum.Why[10];
		const int64 Playable = Closed - Design;
		AddInfo(FString::Printf(TEXT("POSTRACE bar arm=%s level=%s | crossings the rule means to give (not a sting, an opposing puck in flight off the blade, a mate's pass wide of his blade, a stripped man's cooldown, a stumbling carrier's puck) %lld: taken %.1f %% (by him %lld, by a nearer man %lld), nobody's %lld | carry frames outside spins and dekes %lld: within 15 cm of the blade %.2f %%"),
			*Arm, *Level, (long long)Playable, Playable ? 100.0 * (Sum.Taken + Sum.ByOther) / Playable : 0.0, (long long)Sum.Taken, (long long)Sum.ByOther, (long long)(Sum.NotTaken - Design),
			(long long)Sum.CarryPlain, Sum.CarryPlain ? 100.0 * Sum.CarryPlainNear / Sum.CarryPlain : 0.0));
	}
	AddInfo(FString::Printf(TEXT("POSTRACE longshots arm=%s level=%s | opponents' shots from 20-35 m %lld: blocked by a body or stick of ours %lld, the crease man %lld, goals %lld, other (wide, the boards, their own) %lld"), *Arm, *Level, (long long)Sum.LongShots, (long long)Sum.LongBlocked, (long long)Sum.LongCrease, (long long)Sum.LongGoal, (long long)Sum.LongOther));
	Sum.Hands.KeySort(TLess<FString>());
	for (const TPair<FString, int64>& H : Sum.Hands) AddInfo(FString::Printf(TEXT("POSTRACE hands arm=%s level=%s %s %lld"), *Arm, *Level, *H.Key, (long long)H.Value));
	for (int32 w = 0; w < 12; ++w) if (Sum.Why[w]) AddInfo(FString::Printf(TEXT("POSTRACE why-not arm=%s level=%s %s %lld"), *Arm, *Level, WhyName(w), (long long)Sum.Why[w]));
	return true;
}
}

#endif
