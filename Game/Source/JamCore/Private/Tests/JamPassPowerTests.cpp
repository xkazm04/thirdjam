// Lane A207 (track P1, the night of 14 Nov, docs/experiments/PASS-SHOT.md): the pass and the shot as a player feels them. Measurements, outside the gate.
//   Bar.Pass.Measure   every pass in real matches (the scripted regular player against the brain): its speed at release and after 5 / 10 / 20 / 30 m,
//                      where it falls under a skater's 11 m/s and under the blade's 6 m/s, how close it comes to the man it was meant for (blade and body),
//                      the angle between the release and where that man was when the puck passed him, and who touched it first (the man, another mate,
//                      an opponent, nobody). By class: human -> mate, bot -> human, bot -> bot. And every shot's release speed by its charge.
//   env JAM_PASS_MEASURE = "level=pro;player=regular;seeds=40;from=1;rows=0" (every part optional; level may be a list rookie,pro)
#include "Misc/AutomationTest.h"
#include "JamCartridge.h"
#include "JamSim.h"
#include "JamEventBus.h"
#include "JamBrain.h"
#include "JamSimAdapter.h"
#include "JamPlaytester.h"
#include "DifficultyResolve.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamPassPowerTests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

	// One pass in flight, from the release to its first touch by anybody but the passer (or the whistle, or its death on the ice).
	struct FFlight
	{
		int32 Passer = -1, Recv = -1, Team = 0;
		bool bHumanPasser = false, bSaucer = false;
		int64 Start = 0;
		double V0 = 0, D0 = 0, Path = 0;
		FVector2D From = FVector2D::ZeroVector, Dir = FVector2D::ZeroVector, Last = FVector2D::ZeroVector;
		double VAt[4] = {-1, -1, -1, -1};   // speed after 5 / 10 / 20 / 30 m of path (-1 = never got there)
		double PathUnderSkater = -1, PathUnder6 = -1;
		double MinBlade = 1e9, MinBody = 1e9, VAtMin = 0, AngAtMin = 0, PathAtMin = 0;
		int32 RecvReceivedAtStart = 0;
	};
	struct FRow
	{
		int32 Class = 0;   // 0 human -> mate, 1 bot -> human, 2 bot -> bot
		int32 Out = 0;     // 0 the man (caught: a reception), 1 the man (a touch, not seated), 2 another mate, 3 an opponent, 4 nobody (died / whistle)
		FFlight F;
		double VEnd = 0;
		int32 Frames = 0;
	};
	const TCHAR* ClassName(int32 C) { return C == 0 ? TEXT("human_to_mate") : (C == 1 ? TEXT("bot_to_human") : TEXT("bot_to_bot")); }
	const TCHAR* OutName(int32 O) { static const TCHAR* N[] = {TEXT("caught"), TEXT("touched"), TEXT("other_mate"), TEXT("opponent"), TEXT("nobody")}; return N[FMath::Clamp(O, 0, 4)]; }

	struct FShotRow { bool bHuman = false; float Charge = 0.f; double V = 0; };

	struct FTrace
	{
		JamAI::FJamSimAdapter* Sim = nullptr;
		int64 LastFrame = -1;
		bool bOpen = false;
		FFlight F;
		TArray<FRow> Rows;
		TArray<FShotRow> Shots;
		// The puck's integrity: frames it lay outside the boards; substeps... frames in which it moved through a standing body it never touched.
		int32 OutFrames = 0, Through = 0, FastFrames = 0;
		FVector2D LastPuck = FVector2D::ZeroVector;
		int32 LastToucher = -1;
		bool bHaveLast = false;
	};

	int32 ParseRecv(const FName& Kind)
	{
		const FString K = Kind.ToString();
		int32 At = K.Find(TEXT("to_"), ESearchCase::CaseSensitive, ESearchDir::FromEnd);
		return At >= 0 ? FCString::Atoi(*K.Mid(At + 3)) : -1;
	}

	void Close(FTrace& T, int32 Out, double VEnd, int64 Now, const UJamSim& Sim)
	{
		FRow R;
		R.F = T.F;
		const TArray<FJamSkaterState>& Sk = Sim.GetSkaters();
		R.Class = T.F.bHumanPasser ? 0 : ((Sk.IsValidIndex(T.F.Recv) && Sk[T.F.Recv].bHuman) ? 1 : 2);
		R.Out = Out;
		R.VEnd = VEnd;
		R.Frames = static_cast<int32>(Now - T.F.Start);
		T.Rows.Add(R);
		T.bOpen = false;
	}

	void Trace(void* Ctx, const JamAI::FJamAIWorld&, const JamAI::FJamAIInput*, const JamAI::FJamBrain&, int64_t, bool)
	{
		FTrace& T = *static_cast<FTrace*>(Ctx);
		const UJamSim* Sim = T.Sim->GetSim();
		if (!Sim || Sim->GetFrame() == T.LastFrame) return;
		T.LastFrame = Sim->GetFrame();
		const int64 Now = Sim->GetFrame();
		const TArray<FJamSkaterState>& Sk = Sim->GetSkaters();
		const FJamPuckState& P = Sim->GetPuck();
		const double V = P.Vel.Size();
		{
			// Integrity (the doubled shot at 60 Hz with 4 substeps): outside the boards; or through a body: the frame's path passes within 0.3 m of a standing
			// skater's centre (the body radius is 0.4 + the puck's 0.12) under body height while the toucher did not change and it was not that skater's puck.
			FVector2D Nrm;
			if (Sim->BoardDistance(P.Pos, Nrm) > 0.02) ++T.OutFrames;
			if (T.bHaveLast && P.LastTouch == T.LastToucher && P.Z < 0.35 && Sim->GetPhase() == EJamSimPhase::Playing && V > 20.0)
			{
				++T.FastFrames;
				const FVector2D Seg = P.Pos - T.LastPuck;
				const double L2 = Seg.SizeSquared();
				for (const FJamSkaterState& S : Sk)
				{
					if (S.Index == P.LastTouch || S.InBox() || S.IsDown() || L2 < 1e-6) continue;
					const double Tt = FMath::Clamp(FVector2D::DotProduct(S.Pos - T.LastPuck, Seg) / L2, 0.0, 1.0);
					if (FVector2D::Distance(T.LastPuck + Seg * Tt, S.Pos) < 0.3) { ++T.Through; break; }
				}
			}
			T.LastPuck = P.Pos; T.LastToucher = P.LastTouch; T.bHaveLast = true;
		}
		// The flight before this frame's events: a touch by somebody else ends it.
		if (T.bOpen)
		{
			if (Sim->GetPhase() != EJamSimPhase::Playing) Close(T, 4, V, Now, *Sim);
			else if (P.LastTouch != T.F.Passer && !Sk[T.F.Passer].bPassThisFrame)
			{
				const int32 Who = P.LastTouch;
				int32 Out = 3;
				if (Who >= 0 && Who == T.F.Recv && Sk.IsValidIndex(Who)) Out = Sk[Who].PassesReceived > T.F.RecvReceivedAtStart ? 0 : 1;
				else if (Sk.IsValidIndex(Who) && Sk[Who].Team == T.F.Team) Out = 2;
				else if (!Sk.IsValidIndex(Who)) Out = 4;
				Close(T, Out, V, Now, *Sim);
			}
			else
			{
				FFlight& F = T.F;
				F.Path += FVector2D::Distance(F.Last, P.Pos);
				F.Last = P.Pos;
				static const double Marks[4] = {5, 10, 20, 30};
				for (int32 k = 0; k < 4; ++k) if (F.VAt[k] < 0 && F.Path >= Marks[k]) F.VAt[k] = V;
				if (F.PathUnderSkater < 0 && V < 11.0) F.PathUnderSkater = F.Path;
				if (F.PathUnder6 < 0 && V < 6.0) F.PathUnder6 = F.Path;
				if (Sk.IsValidIndex(F.Recv))
				{
					const FJamSkaterState& R = Sk[F.Recv];
					const double Db = FVector2D::Distance(P.Pos, R.BladePos);
					F.MinBody = FMath::Min(F.MinBody, FVector2D::Distance(P.Pos, R.Pos));
					if (Db < F.MinBlade)
					{
						F.MinBlade = Db; F.VAtMin = V; F.PathAtMin = F.Path;
						const FVector2D ToR = (R.BladePos - F.From).GetSafeNormal();
						F.AngAtMin = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector2D::DotProduct(ToR, F.Dir), -1.0, 1.0)));
					}
				}
				if (V < 0.3 || Now - F.Start > 420) Close(T, 4, V, Now, *Sim);
			}
		}
		// This frame's releases.
		for (const FJamSkaterState& S : Sk)
		{
			if (S.bShotThisFrame) { FShotRow Sh; Sh.bHuman = S.bHuman; Sh.Charge = S.LastShotCharge; Sh.V = V; T.Shots.Add(Sh); }
			if (!S.bPassThisFrame) continue;
			if (T.bOpen) Close(T, 4, V, Now, *Sim);
			FFlight F;
			F.Passer = S.Index; F.Team = S.Team; F.bHumanPasser = S.bHuman; F.Start = Now; F.V0 = V; F.From = P.Pos; F.Last = P.Pos; F.Dir = P.Vel.GetSafeNormal();
			F.bSaucer = S.bSaucerThisFrame;
			for (const FJamSimEvent& E : Sim->GetFrameEvents()) if (E.Name == JamEvents::Pass && E.Who == S.Index) F.Recv = ParseRecv(E.Kind);
			if (Sk.IsValidIndex(F.Recv)) { F.D0 = FVector2D::Distance(P.Pos, Sk[F.Recv].Pos); F.RecvReceivedAtStart = Sk[F.Recv].PassesReceived; }
			T.F = F;
			T.bOpen = true;
		}
	}

	struct FOpt { FString Levels = TEXT("pro"), Player = TEXT("regular"), Rink = TEXT("hockey.regular"); int32 Seeds = 40, Rows = 0, From = 1; };
	FOpt ReadOpt()
	{
		FOpt O;
		TArray<FString> Parts;
		FPlatformMisc::GetEnvironmentVariable(TEXT("JAM_PASS_MEASURE")).ParseIntoArray(Parts, TEXT(";"));
		for (const FString& Part : Parts)
		{
			FString K, V;
			if (!Part.Split(TEXT("="), &K, &V)) continue;
			if (K == TEXT("level")) O.Levels = V; else if (K == TEXT("player")) O.Player = V; else if (K == TEXT("seeds")) O.Seeds = FCString::Atoi(*V);
			else if (K == TEXT("rows")) O.Rows = FCString::Atoi(*V); else if (K == TEXT("from")) O.From = FCString::Atoi(*V); else if (K == TEXT("rink")) O.Rink = V;
		}
		return O;
	}
	double Median(TArray<double> A) { if (A.Num() == 0) return -1; A.Sort(); return A[A.Num() / 2]; }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBarPassMeasure, "Bar.Pass.Measure", JamPassPowerTests::Flags)
bool FJamBarPassMeasure::RunTest(const FString& Parameters)
{
	using namespace JamPassPowerTests;
	const FOpt O = ReadOpt();
	TArray<FString> Errors;
	UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(O.Rink), Errors);
	for (const FString& E : Errors) AddError(E);
	if (!C) return false;
	const JamAI::EPlaytester Kind = O.Player == TEXT("novice") ? JamAI::EPlaytester::Novice : (O.Player == TEXT("good") ? JamAI::EPlaytester::Good : JamAI::EPlaytester::Regular);
	TArray<FString> Levels;
	O.Levels.ParseIntoArray(Levels, TEXT(","));
	for (const FString& Level : Levels)
	{
		TArray<FRow> All;
		TArray<FShotRow> Shots;
		int32 Wins = 0, OutFrames = 0, Through = 0, FastFrames = 0;
		for (int32 Seed = O.From; Seed < O.From + O.Seeds; ++Seed)
		{
			UJamCartridge* M = DuplicateObject<UJamCartridge>(C, GetTransientPackage());
			JamAI::FJamSimAdapter Sim(*M);
			FTrace* T = new FTrace();
			T->Sim = &Sim;
			const JamAI::FJamMatchConfig Cfg = JamAI::ToMatchConfigResolved(*M, *Level);
			const JamAI::FJamPlaytestStats S = JamAI::RunPlaytestMatch(Sim, Cfg, static_cast<uint64>(Seed), Kind, 36000, M->Game.Shot.ChargeFramesMax, false, &Trace, T, 1);
			if (S.GoalsFor > S.GoalsAgainst) ++Wins;
			All.Append(T->Rows);
			Shots.Append(T->Shots);
			OutFrames += T->OutFrames; Through += T->Through; FastFrames += T->FastFrames;
			delete T;
		}
		int32 Shown = 0;
		for (const FRow& R : All)
		{
			if (Shown++ >= O.Rows) break;
			AddInfo(FString::Printf(TEXT("PASSROW level=%s class=%s out=%s v0=%.1f d0=%.1f v5=%.1f v10=%.1f v20=%.1f v30=%.1f under11=%.1f under6=%.1f minblade=%.2f minbody=%.2f vmin=%.1f pathmin=%.1f ang=%.1f path=%.1f vend=%.1f frames=%d saucer=%d"),
				*Level, ClassName(R.Class), OutName(R.Out), R.F.V0, R.F.D0, R.F.VAt[0], R.F.VAt[1], R.F.VAt[2], R.F.VAt[3], R.F.PathUnderSkater, R.F.PathUnder6, R.F.MinBlade, R.F.MinBody,
				R.F.VAtMin, R.F.PathAtMin, R.F.AngAtMin, R.F.Path, R.VEnd, R.Frames, R.F.bSaucer ? 1 : 0));
		}
		for (int32 Cl = 0; Cl < 3; ++Cl)
		{
			int32 N = 0, NoMan = 0, Out[5] = {0, 0, 0, 0, 0}, Near = 0, DiedBefore = 0, Far = 0, FarCaught = 0, Ang20 = 0, Ang20Caught = 0;
			TArray<double> V0, D0, MinB, Ang, U6, VMin;
			for (const FRow& R : All)
			{
				if (R.Class != Cl) continue;
				if (R.F.Recv < 0) { ++NoMan; continue; }   // a pass along the stick with no mate in the cone: counted apart
				++N; ++Out[R.Out];
				V0.Add(R.F.V0); D0.Add(R.F.D0); MinB.Add(R.F.MinBlade); Ang.Add(R.F.AngAtMin); VMin.Add(R.F.VAtMin);
				if (R.F.PathUnder6 >= 0) U6.Add(R.F.PathUnder6);
				if (R.F.MinBlade < 0.6) ++Near;
				if (R.F.MinBlade < 1.5 && R.F.VAtMin < 6.0) ++DiedBefore;
				if (R.F.D0 >= 15.0) { ++Far; if (R.Out <= 1) ++FarCaught; }
				if (R.F.AngAtMin <= 20.0) { ++Ang20; if (R.Out <= 1) ++Ang20Caught; }
			}
			const double Nn = FMath::Max(1, N);
			AddInfo(FString::Printf(TEXT("PASSSUM level=%s class=%s n=%d no_man=%d per_match=%.1f | caught=%.1f%% touched=%.1f%% other_mate=%.1f%% opponent=%.1f%% nobody=%.1f%% | median v0=%.1f d0=%.1f minblade=%.2f ang=%.1f v_at_min=%.1f under6_at=%.1f | blade_within_0.6=%.1f%% slow_at_him(<6)=%.1f%% | d0>=15m: %d, reached %.1f%% | ang<=20: %d, reached %.1f%%"),
				*Level, ClassName(Cl), N, NoMan, N / static_cast<double>(FMath::Max(1, O.Seeds)), 100.0 * Out[0] / Nn, 100.0 * Out[1] / Nn, 100.0 * Out[2] / Nn, 100.0 * Out[3] / Nn, 100.0 * Out[4] / Nn,
				Median(V0), Median(D0), Median(MinB), Median(Ang), Median(VMin), Median(U6), 100.0 * Near / Nn, 100.0 * DiedBefore / Nn, Far, 100.0 * FarCaught / FMath::Max(1, Far), Ang20, 100.0 * Ang20Caught / FMath::Max(1, Ang20)));
		}
		for (int32 H = 0; H < 2; ++H)
		{
			TArray<double> Tap, Half, Full;
			for (const FShotRow& Sh : Shots)
			{
				if (Sh.bHuman != (H == 1)) continue;
				if (Sh.Charge <= 0.f) Tap.Add(Sh.V); else if (Sh.Charge < 0.99f) Half.Add(Sh.V); else Full.Add(Sh.V);
			}
			AddInfo(FString::Printf(TEXT("SHOTSUM level=%s who=%s tap n=%d median %.1f | part n=%d median %.1f | full n=%d median %.1f"), *Level, H ? TEXT("human") : TEXT("bots"),
				Tap.Num(), Median(Tap), Half.Num(), Median(Half), Full.Num(), Median(Full)));
		}
		AddInfo(FString::Printf(TEXT("PASSMATCHES level=%s seeds=%d wins=%d | puck outside the boards %d frames, through a standing body %d of %d frames over 20 m/s (bar 0, 0)"), *Level, O.Seeds, Wins, OutFrames, Through, FastFrames));
	}
	return true;
}

#endif
