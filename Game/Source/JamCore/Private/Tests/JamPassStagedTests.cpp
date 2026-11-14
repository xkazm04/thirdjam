// Lane A207 (track P1, the night of 14 Nov, docs/experiments/PASS-SHOT.md): the pass and the shot in staged scenes.
//   Bar.Pass.Staged   200 staged passes of the human on hockey.regular (every other skater parked far away unless the case puts an opponent in the lane):
//                       tap   40 taps at an open, standing mate 7..10 m away, the stick on him: caught share (bar 95 %)
//                       full  20 full holds (30 frames) along the ice with nobody there: the way to 7 m/s and the speed at 30 m (bar >= 30 m, > 6 m/s)
//                       aim   100 passes with the stick 0..20 degrees off a mate 10..25 m away standing or skating across at 0..8 m/s, hold by distance:
//                             caught share (bar 80 %)
//                       long  20 full holds at a mate 28..32 m away: caught share and arrival speed
//                       hold  the release speed by hold frames 0..36 (the power curve)
//   Bar.Shot.Staged   the human's shot speed at tap, half and full charge (bar 2.0 x the neutral's), and 200 full shots at a standing body, the boards and the
//                     posts from 2..20 m: none through a body or out of the rink (no tunnelling at 60 Hz with 4 puck substeps)
//   env JAM_PASS_STAGED = "set=neutral" plays the neutral values (game.pass.power.full_frames 0, aim.cone_deg 0, catch_m 0, clear_own_body false, shot.speed_scale 1).
#include "Misc/AutomationTest.h"
#include "JamCartridge.h"
#include "JamSim.h"
#include "JamEventBus.h"
#include "JamAICore.h"
#include "JamSimAdapter.h"
#include "DifficultyResolve.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace JamPassStagedTests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter;

	void Neutral(FJamGameConfig& G)
	{
		G.Pass.PowerFullFrames = 0; G.Pass.AimConeDeg = 0.0; G.Pass.CatchM = 0.0; G.Pass.bClearOwnBody = false; G.Shot.SpeedScale = 1.0;
	}

	struct FPassStage
	{
		TUniquePtr<JamAI::FJamSimAdapter> Adapter;
		JamAI::FJamMatchConfig Cfg;
		JamAI::FJamAIWorld W;
		JamAI::FJamAIInput In[JamAI::kMaxSkaters];
		UJamSim* Sim = nullptr;
		int32 Me = -1, Mate = -1;

		FPassStage(const UJamCartridge& Cart, const FJamGameConfig& G)
		{
			Adapter = MakeUnique<JamAI::FJamSimAdapter>(G);
			Cfg = JamAI::ToMatchConfigResolved(Cart, TEXT("pro"));
			Adapter->Reset(Cfg, 1);
			Sim = Adapter->GetSim();
			Sim->SkipFaceoff();
			Me = Sim->GetHumanIndex();
			// Park everybody far away: own team behind at -38, the other team in the far corners.
			int32 k = 0;
			for (const FJamSkaterState& S : Sim->GetSkaters())
			{
				if (S.Index == Me) continue;
				if (S.Team == Sim->GetSkaters()[Me].Team) { if (Mate < 0) Mate = S.Index; Sim->SetSkater(S.Index, FVector2D(-38.0, -15.0 + 7.5 * (k++ % 5)), FVector2D::ZeroVector, 0.0); }
				else Sim->SetSkater(S.Index, FVector2D(38.0, -18.0 + 9.0 * (k++ % 5)), FVector2D::ZeroVector, UE_DOUBLE_PI);
			}
		}

		void Step()
		{
			Adapter->View(W);
			Adapter->Step(In);
		}
		void Clear() { for (JamAI::FJamAIInput& I : In) I = JamAI::FJamAIInput(); }
		// The human at P facing +x, the puck on his blade, stick at rest; a few quiet frames so he carries it.
		void GivePuck(const FVector2D& P, const FVector2D& StickDir)
		{
			Sim->SetSkater(Me, P, FVector2D::ZeroVector, 0.0);
			Clear();
			In[Me].Stick = JamAI::FVec2(StickDir.X, StickDir.Y);
			for (int32 i = 0; i < 2; ++i)
			{
				const FJamSkaterState& S = Sim->GetSkaters()[Me];
				Sim->SetPuck(S.BladePos, FVector2D::ZeroVector);
				Step();
			}
		}
		// Hold pass for Hold frames (at least 1) with the stick on StickDir, then let go; returns the frame after the release.
		void Pass(int32 Hold, const FVector2D& StickDir, const TFunctionRef<void()>& Each)
		{
			for (int32 f = 0; f < Hold + 1; ++f)
			{
				Clear();
				In[Me].Stick = JamAI::FVec2(StickDir.X, StickDir.Y);
				In[Me].bPassHeld = f < Hold;
				Each();
				Step();
			}
		}
	};

	struct FFlightOut { bool bCaught = false; double MinBlade = 1e9, VAtMin = 0, Way = 0, VAt30 = -1, WayTo7 = -1; int32 Frames = 0; };

	// Steps up to MaxF frames after a pass, every frame calling Each (the receiver's script); measures the puck against Mate.
	FFlightOut Fly(FPassStage& Sc, int32 MaxF, const TFunctionRef<void()>& Each)
	{
		FFlightOut O;
		const int32 R0 = Sc.Sim->GetSkaters()[Sc.Mate].PassesReceived;
		FVector2D Last = Sc.Sim->GetPuck().Pos;
		for (int32 f = 0; f < MaxF; ++f)
		{
			Sc.Clear();
			Each();
			const double VBefore = Sc.Sim->GetPuck().Vel.Size();
			Sc.Step();
			const FJamPuckState& P = Sc.Sim->GetPuck();
			const double V = P.Vel.Size();
			O.Way += FVector2D::Distance(Last, P.Pos);
			Last = P.Pos;
			if (O.VAt30 < 0 && O.Way >= 30.0) O.VAt30 = V;
			if (O.WayTo7 < 0 && V < 7.0) O.WayTo7 = O.Way;
			const double Db = FVector2D::Distance(P.Pos, Sc.Sim->GetSkaters()[Sc.Mate].BladePos);
			if (Db < O.MinBlade) { O.MinBlade = Db; O.VAtMin = V; }
			if (Sc.Sim->GetSkaters()[Sc.Mate].PassesReceived > R0) { O.bCaught = true; O.Frames = f; O.VAtMin = VBefore; break; }
			if (P.LastTouch != Sc.Me && P.LastTouch != Sc.Mate) break;
			if (V < 0.2) break;
		}
		return O;
	}
	FVector2D Rot(const FVector2D& V, double Deg) { const double R = FMath::DegreesToRadians(Deg); return FVector2D(V.X * FMath::Cos(R) - V.Y * FMath::Sin(R), V.X * FMath::Sin(R) + V.Y * FMath::Cos(R)); }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBarPassStaged, "Bar.Pass.Staged", JamPassStagedTests::Flags)
bool FJamBarPassStaged::RunTest(const FString& Parameters)
{
	using namespace JamPassStagedTests;
	TArray<FString> Errors;
	UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")), Errors);
	if (!C) return false;
	const bool bNeutral = FPlatformMisc::GetEnvironmentVariable(TEXT("JAM_PASS_STAGED")).Contains(TEXT("neutral"));
	FJamGameConfig G = C->Game;
	if (bNeutral) Neutral(G);
	const TCHAR* Arm = bNeutral ? TEXT("neutral") : TEXT("on");
	{
		FPassStage Sc(*C, G);
		AddInfo(FString::Printf(TEXT("STAGEDINFO arm=%s full_speed=%.2f tap_speed=%.2f speed_after_30m_of_full=%.2f"), Arm, Sc.Sim->PassFullSpeed(), Sc.Sim->PassSpeedForHold(0), Sc.Sim->PuckSpeedAfter(Sc.Sim->PassFullSpeed(), 30.0)));
	}
	// hold: the release speed by hold frames.
	{
		FString Row;
		for (int32 H = 1; H <= 36; H += (H < 6 ? 1 : 5))
		{
			FPassStage Sc(*C, G);
			Sc.GivePuck(FVector2D(-20.0, 0.0), FVector2D(1.0, 0.0));
			Sc.Pass(H, FVector2D(1.0, 0.0), [] {});
			Row += FString::Printf(TEXT(" %d:%.1f"), H, Sc.Sim->GetPuck().Vel.Size());
		}
		AddInfo(FString::Printf(TEXT("STAGEDHOLD arm=%s frames:speed%s"), Arm, *Row));
	}
	// tap: 40 taps at a standing mate 7..10 m.
	int32 TapN = 0, TapC = 0;
	for (int32 i = 0; i < 40; ++i)
	{
		FPassStage Sc(*C, G);
		const double D = 7.0 + 3.0 * (i % 4) / 3.0;
		const double Ang = -60.0 + 120.0 * (i / 4) / 9.0;   // where he stands around the passer
		const FVector2D From(-10.0, 0.0);
		const FVector2D Dir = Rot(FVector2D(1.0, 0.0), Ang);
		Sc.Sim->SetSkater(Sc.Mate, From + Dir * D, FVector2D::ZeroVector, FMath::Atan2(-Dir.Y, -Dir.X));
		Sc.GivePuck(From, Dir);
		Sc.Pass(2, Dir, [] {});
		const FFlightOut O = Fly(Sc, 120, [] {});
		++TapN; if (O.bCaught) ++TapC;
		if (!O.bCaught) AddInfo(FString::Printf(TEXT("STAGEDMISS arm=%s case=tap d=%.1f ang=%.0f minblade=%.2f v=%.1f"), Arm, D, Ang, O.MinBlade, O.VAtMin));
	}
	// full: 20 full holds into open ice (nobody there): the way to 7 m/s and the speed at 30 m.
	TArray<double> Ways, V30;
	for (int32 i = 0; i < 20; ++i)
	{
		FPassStage Sc(*C, G);
		const FVector2D Dir = Rot(FVector2D(1.0, 0.0), -10.0 + i);
		Sc.Sim->SetSkater(Sc.Mate, FVector2D(-38.0, 20.0), FVector2D::ZeroVector, 0.0);
		Sc.GivePuck(FVector2D(-25.0, 0.0), Dir);
		Sc.Pass(30, Dir, [] {});
		const FFlightOut O = Fly(Sc, 400, [] {});
		Ways.Add(O.WayTo7 < 0 ? O.Way : O.WayTo7); V30.Add(O.VAt30);
	}
	Ways.Sort(); V30.Sort();
	// aim: 100 passes, the stick 0..20 degrees off a mate 10..25 m away, standing or skating across at 0..8 m/s; the hand follows him and holds until the meter reaches him.
	int32 AimN = 0, AimC = 0;
	for (int32 i = 0; i < 100; ++i)
	{
		FPassStage Sc(*C, G);
		const double D = 10.0 + 15.0 * (i % 5) / 4.0;
		const double Off = (i % 2 ? 1.0 : -1.0) * 20.0 * ((i / 10) % 5) / 4.0;
		const double Sp = 2.0 * ((i / 2) % 5);
		const double Side = (i / 50) ? 1.0 : -1.0;
		const FVector2D From(-20.0, 0.0);
		const FVector2D ToMate = Rot(FVector2D(1.0, 0.0), 30.0 * Side);
		const FVector2D MateAt = From + ToMate * D;
		const FVector2D Across = FVector2D(-ToMate.Y, ToMate.X) * (Side > 0 ? -1.0 : 1.0);
		Sc.Sim->SetSkater(Sc.Mate, MateAt, Across * Sp, FMath::Atan2(Across.Y, Across.X));
		const FVector2D Stick = Rot(ToMate, Off);
		Sc.GivePuck(From, Stick);
		// The hand: the stick Off degrees off the line to where the mate IS (it follows him through the hold, as a person's mouse does), and it lets go when
		// the meter's line (UJamSim::PassReachM at the hold's power, HUDPassMeter) reaches where he will be in 1.5 s (a person holds until the line is at him).
		auto Skate = [&Sc, Across, Sp] { Sc.In[Sc.Mate].Move = JamAI::FVec2(Across.X * Sp / 11.0, Across.Y * Sp / 11.0); };
		int32 Hold = 0;
		for (int32 f = 0; f < 31; ++f)
		{
			const FJamSkaterState& Mt = Sc.Sim->GetSkaters()[Sc.Mate];
			const FVector2D P = Sc.Sim->GetPuck().Pos;
			const FVector2D AimNow = Rot((Mt.Pos - P).GetSafeNormal(), Off);
			const double Want = FMath::Min(30.0, FVector2D::Distance(P, Mt.Pos + Mt.Vel * 1.5));
			const bool bLetGo = f >= 2 && (Sc.Sim->PassReachM(Sc.Sim->PassSpeedForHold(f)) >= Want || f == 30);
			Sc.Clear();
			Sc.In[Sc.Me].Stick = JamAI::FVec2(AimNow.X, AimNow.Y);
			Sc.In[Sc.Me].bPassHeld = !bLetGo;
			Skate();
			Sc.Step();
			if (bLetGo) { Hold = f; break; }
		}
		const FVector2D MateVel = Sc.Sim->GetSkaters()[Sc.Mate].Vel;
		const FVector2D PuckVel = Sc.Sim->GetPuck().Vel;
		const FFlightOut O = Fly(Sc, 240, Skate);
		++AimN; if (O.bCaught) ++AimC;
		if (!O.bCaught) AddInfo(FString::Printf(TEXT("STAGEDMISS arm=%s case=aim d=%.1f off=%.0f sp=%.0f hold=%d minblade=%.2f v=%.1f | release: puck %.1f m/s at %.0f deg, mate vel (%.1f,%.1f), to-mate %.0f deg"), Arm, D, Off, Sp, Hold, O.MinBlade, O.VAtMin,
			PuckVel.Size(), FMath::RadiansToDegrees(FMath::Atan2(PuckVel.Y, PuckVel.X)), MateVel.X, MateVel.Y, FMath::RadiansToDegrees(FMath::Atan2(ToMate.Y, ToMate.X))));
	}
	// long: 20 full holds at a mate 28..32 m away, standing.
	int32 LongN = 0, LongC = 0;
	TArray<double> LongV;
	for (int32 i = 0; i < 20; ++i)
	{
		FPassStage Sc(*C, G);
		const double D = 28.0 + 4.0 * (i % 5) / 4.0;
		const FVector2D Dir = Rot(FVector2D(1.0, 0.0), -20.0 + 2.0 * i);
		const FVector2D From(-30.0, 0.0);
		Sc.Sim->SetSkater(Sc.Mate, From + Dir * D, FVector2D::ZeroVector, FMath::Atan2(-Dir.Y, -Dir.X));
		Sc.GivePuck(From, Dir);
		Sc.Pass(30, Dir, [] {});
		const FFlightOut O = Fly(Sc, 300, [] {});
		++LongN; if (O.bCaught) { ++LongC; LongV.Add(O.VAtMin); }
		else AddInfo(FString::Printf(TEXT("STAGEDMISS arm=%s case=long d=%.1f minblade=%.2f v=%.1f way=%.1f"), Arm, D, O.MinBlade, O.VAtMin, O.Way));
	}
	LongV.Sort();
	AddInfo(FString::Printf(TEXT("STAGEDSUM arm=%s | tap at 7-10 m caught %d/%d = %.1f%% (bar 95) | full hold: way to 7 m/s median %.1f m (min %.1f), speed at 30 m median %.1f (min %.1f) (bar >= 30 m, > 6) | aim 0-20 deg off, 10-25 m, 0-8 m/s across: caught %d/%d = %.1f%% (bar 80) | long 28-32 m full: caught %d/%d, arrival median %.1f m/s"),
		Arm, TapC, TapN, 100.0 * TapC / FMath::Max(1, TapN), Ways[Ways.Num() / 2], Ways[0], V30[V30.Num() / 2], V30[0], AimC, AimN, 100.0 * AimC / FMath::Max(1, AimN), LongC, LongN, LongV.Num() ? LongV[LongV.Num() / 2] : -1.0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBarShotStaged, "Bar.Shot.Staged", JamPassStagedTests::Flags)
bool FJamBarShotStaged::RunTest(const FString& Parameters)
{
	using namespace JamPassStagedTests;
	TArray<FString> Errors;
	UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")), Errors);
	if (!C) return false;
	double Sp[2][3] = {};
	for (int32 Arm = 0; Arm < 2; ++Arm)
	{
		FJamGameConfig G = C->Game;
		if (Arm == 0) Neutral(G);
		const int32 Holds[3] = {2, 8, 30};
		for (int32 k = 0; k < 3; ++k)
		{
			FPassStage Sc(*C, G);
			Sc.GivePuck(FVector2D(0.0, 0.0), FVector2D(1.0, 0.0));
			for (int32 f = 0; f < Holds[k] + 1; ++f) { Sc.Clear(); Sc.In[Sc.Me].Stick = JamAI::FVec2(1.0, 0.0); Sc.In[Sc.Me].bShootHeld = f < Holds[k]; Sc.Step(); }
			Sp[Arm][k] = Sc.Sim->GetPuck().Vel.Size();
		}
	}
	AddInfo(FString::Printf(TEXT("SHOTSTAGE speed neutral tap %.1f half %.1f full %.1f | on tap %.1f half %.1f full %.1f | ratio %.2f %.2f %.2f (bar 2.0)"),
		Sp[0][0], Sp[0][1], Sp[0][2], Sp[1][0], Sp[1][1], Sp[1][2], Sp[1][0] / FMath::Max(0.1, Sp[0][0]), Sp[1][1] / FMath::Max(0.1, Sp[0][1]), Sp[1][2] / FMath::Max(0.1, Sp[0][2])));
	// Tunnelling: full shots on the ice (lift off) at a standing body, at the boards, at a post, from 2..20 m: the puck must never be found past the body
	// (beyond its centre on the far side while it never touched it), outside the boards, or inside a post.
	FJamGameConfig G = C->Game;
	G.Shot.LiftTapM = G.Shot.LiftHalfM = G.Shot.LiftFullM = 0.0;
	int32 N = 0, Through = 0, Out = 0;
	for (int32 i = 0; i < 200; ++i)
	{
		FPassStage Sc(*C, G);
		const int32 Kind = i % 3;   // 0 a body, 1 the boards, 2 the far post
		const double D = 2.0 + 18.0 * ((i / 3) % 10) / 9.0;
		const double Y = -0.3 + 0.6 * ((i / 30) % 7) / 6.0;
		const FVector2D From(Kind == 1 ? 20.0 : Sc.Sim->GoalLineX() - D - 1.0, Kind == 2 ? 2.0 + Y : Y);
		const int32 Opp = [&] { for (const FJamSkaterState& S : Sc.Sim->GetSkaters()) if (S.Team != Sc.Sim->GetSkaters()[Sc.Me].Team) return S.Index; return -1; }();
		if (Kind == 0) Sc.Sim->SetSkater(Opp, From + FVector2D(D, 0.0), FVector2D::ZeroVector, UE_DOUBLE_PI);
		Sc.GivePuck(From, FVector2D(1.0, 0.0));
		for (int32 f = 0; f < 31; ++f) { Sc.Clear(); Sc.In[Sc.Me].Stick = JamAI::FVec2(1.0, 0.0); Sc.In[Sc.Me].bShootHeld = f < 30; Sc.Step(); }
		bool bThrough = false, bOut = false;
		for (int32 f = 0; f < 120; ++f)
		{
			Sc.Clear(); Sc.Step();
			const FJamPuckState& P = Sc.Sim->GetPuck();
			FVector2D Nrm;
			if (Sc.Sim->BoardDistance(P.Pos, Nrm) > 0.01) bOut = true;
			if (Kind == 0 && P.LastTouch == Sc.Me && P.Pos.X > Sc.Sim->GetSkaters()[Opp].Pos.X + 0.1 && FMath::Abs(P.Pos.Y - Sc.Sim->GetSkaters()[Opp].Pos.Y) < 0.4) bThrough = true;
		}
		++N; if (bThrough) ++Through; if (bOut) ++Out;
	}
	AddInfo(FString::Printf(TEXT("SHOTSTAGE tunnel: %d full shots on the ice at a body / the boards / a post from 2-20 m: through a body %d, outside the boards %d (bar 0, 0)"), N, Through, Out));
	return true;
}

// Lane A207: the rules in the gate. Staged on hockey.regular: the keys load and ship; the power curve (tap 14, full solved to carry 30 m and still move
// 6+ m/s there); a full hold at a standing mate 30 m away is caught; the cone sends a pass whose stick points 15 degrees off a mate to him, and one
// pointing 40 degrees off along the stick; the doubled shot (2.0 x at tap and full); the own body lets the pass by; and the neutral values give the
// pass before the lane (pass_speed 14 whatever the hold, no catch beyond the 0.6 m disc).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPassPower, "Jam.Core.PassPower", JamPassStagedTests::Flags)
bool FJamPassPower::RunTest(const FString& Parameters)
{
	using namespace JamPassStagedTests;
	TArray<FString> Errors;
	UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")), Errors);
	for (const FString& E : Errors) AddError(E);
	if (!C) return false;
	const FJamGameConfig& G = C->Game;
	TestEqual(TEXT("pass.power.full_frames ships 30"), G.Pass.PowerFullFrames, 30);
	TestEqual(TEXT("pass.aim.cone_deg ships 25"), G.Pass.AimConeDeg, 25.0);
	TestEqual(TEXT("pass.catch_m ships 1.2"), G.Pass.CatchM, 1.2);
	TestTrue(TEXT("pass.clear_own_body ships true"), G.Pass.bClearOwnBody);
	TestEqual(TEXT("shot.speed_scale ships 2"), G.Shot.SpeedScale, 2.0);
	{
		// The keys load when written (and an unknown key under pass is still refused by the schema).
		const FString Raw = C->RawJson;
		UJamCartridge* V = UJamCartridge::LoadFromString(Raw.Replace(TEXT("\"puck\": {"), TEXT("\"pass\": {\"power\": {\"full_frames\": 20, \"reach_share\": 0.5, \"arrive_mps\": 9}, \"aim\": {\"cone_deg\": 10}, \"catch_m\": 0.8, \"clear_own_body\": false}, \"puck\": {")), Errors);
		if (TestNotNull(TEXT("the pass keys load"), V))
		{
			TestEqual(TEXT("full_frames read"), V->Game.Pass.PowerFullFrames, 20); TestEqual(TEXT("reach_share read"), V->Game.Pass.PowerReachShare, 0.5);
			TestEqual(TEXT("arrive_mps read"), V->Game.Pass.PowerArriveMps, 9.0); TestEqual(TEXT("cone_deg read"), V->Game.Pass.AimConeDeg, 10.0);
			TestEqual(TEXT("catch_m read"), V->Game.Pass.CatchM, 0.8); TestFalse(TEXT("clear_own_body read"), V->Game.Pass.bClearOwnBody);
		}
		TArray<FString> E2;
		UJamCartridge* Bad = UJamCartridge::LoadFromString(Raw.Replace(TEXT("\"puck\": {"), TEXT("\"pass\": {\"power\": {\"full_frame\": 20}}, \"puck\": {")), E2);
		TestTrue(TEXT("an unknown key under pass.power is refused"), Bad == nullptr || E2.Num() > 0);
	}
	// The power curve and the full pass's reach.
	{
		FPassStage Sc(*C, G);
		TestEqual(TEXT("a tap is pass_speed"), Sc.Sim->PassSpeedForHold(2), 14.0, 1e-6);
		TestTrue(TEXT("power grows with the hold"), Sc.Sim->PassSpeedForHold(10) > 14.5 && Sc.Sim->PassSpeedForHold(20) > Sc.Sim->PassSpeedForHold(10));
		TestTrue(TEXT("the full speed carries 30 m at arrive_mps (closed form)"), FMath::Abs(Sc.Sim->PuckSpeedAfter(Sc.Sim->PassFullSpeed(), 30.0) - G.Pass.PowerArriveMps) < 0.05);
		Sc.GivePuck(FVector2D(-25.0, 0.0), FVector2D(1.0, 0.0));
		Sc.Pass(30, FVector2D(1.0, 0.0), [] {});
		const FFlightOut O = Fly(Sc, 400, [] {});
		AddInfo(FString::Printf(TEXT("PASSPOWER full hold into open ice: %.2f m/s at release, %.2f m/s after 30 m, %.1f m to 7 m/s"), Sc.Sim->PassFullSpeed(), O.VAt30, O.WayTo7));
		TestTrue(TEXT("a full hold covers 30 m still over 6 m/s (the operator's third of the rink)"), O.VAt30 > 6.0);
	}
	// A full hold at a standing mate 30 m away is caught.
	{
		FPassStage Sc(*C, G);
		const FVector2D From(-30.0, 0.0);
		Sc.Sim->SetSkater(Sc.Mate, From + FVector2D(30.0, 0.0), FVector2D::ZeroVector, UE_DOUBLE_PI);
		Sc.GivePuck(From, FVector2D(1.0, 0.0));
		Sc.Pass(30, FVector2D(1.0, 0.0), [] {});
		TestTrue(TEXT("a full pass to a standing mate 30 m away is caught"), Fly(Sc, 300, [] {}).bCaught);
	}
	// The cone: 15 degrees off a mate 12 m away -> to him; 40 degrees off -> along the stick (nobody caught it).
	for (int32 k = 0; k < 2; ++k)
	{
		FPassStage Sc(*C, G);
		const FVector2D From(-20.0, 0.0);
		Sc.Sim->SetSkater(Sc.Mate, From + FVector2D(12.0, 0.0), FVector2D::ZeroVector, UE_DOUBLE_PI);
		const FVector2D Stick = Rot(FVector2D(1.0, 0.0), k == 0 ? 15.0 : 40.0);
		Sc.GivePuck(From, Stick);
		Sc.Pass(8, Stick, [] {});
		const FVector2D V = Sc.Sim->GetPuck().Vel;
		const double Deg = FMath::RadiansToDegrees(FMath::Atan2(V.Y, V.X));
		const FFlightOut O = Fly(Sc, 200, [] {});
		if (k == 0) TestTrue(*FString::Printf(TEXT("stick 15 deg off a mate: the pass goes to him (left at %.1f deg) and he takes it"), Deg), FMath::Abs(Deg) < 5.0 && O.bCaught);
		else TestTrue(*FString::Printf(TEXT("stick 40 deg off: the pass goes along the stick (left at %.1f deg)"), Deg), FMath::Abs(Deg - 40.0) < 5.0 && !O.bCaught);
	}
	// The doubled shot.
	{
		double V[2][2] = {};
		for (int32 Arm = 0; Arm < 2; ++Arm)
			for (int32 k = 0; k < 2; ++k)
			{
				FJamGameConfig G2 = G;
				if (Arm == 0) G2.Shot.SpeedScale = 1.0;
				FPassStage Sc(*C, G2);
				Sc.GivePuck(FVector2D(0.0, 0.0), FVector2D(1.0, 0.0));
				const int32 Hold = k == 0 ? 2 : 30;
				for (int32 f = 0; f < Hold + 1; ++f) { Sc.Clear(); Sc.In[Sc.Me].Stick = JamAI::FVec2(1.0, 0.0); Sc.In[Sc.Me].bShootHeld = f < Hold; Sc.Step(); }
				V[Arm][k] = Sc.Sim->GetPuck().Vel.Size();
			}
		TestTrue(*FString::Printf(TEXT("the tap shot doubles (%.1f -> %.1f)"), V[0][0], V[1][0]), FMath::Abs(V[1][0] / FMath::Max(0.1, V[0][0]) - 2.0) < 0.03);
		TestTrue(*FString::Printf(TEXT("the full shot doubles (%.1f -> %.1f)"), V[0][1], V[1][1]), FMath::Abs(V[1][1] / FMath::Max(0.1, V[0][1]) - 2.0) < 0.03);
	}
	// The own body: a bot's pass from the puck ahead of him to the human straight behind him crosses his body: with clear_own_body it reaches the human.
	for (int32 Arm = 0; Arm < 2; ++Arm)
	{
		FJamGameConfig G2 = G;
		G2.Pass.bClearOwnBody = Arm == 1;
		FPassStage Sc(*C, G2);
		const FVector2D From(-10.0, 0.0);
		Sc.Sim->SetSkater(Sc.Mate, From, FVector2D::ZeroVector, 0.0);
		Sc.Sim->SetSkater(Sc.Me, From + FVector2D(-10.0, 0.0), FVector2D::ZeroVector, 0.0);
		Sc.Clear();
		for (int32 i = 0; i < 2; ++i) { Sc.Sim->SetPuck(Sc.Sim->GetSkaters()[Sc.Mate].BladePos, FVector2D::ZeroVector); Sc.Clear(); Sc.Step(); }
		const int32 R0 = Sc.Sim->GetSkaters()[Sc.Me].PassesReceived;
		Sc.Clear(); Sc.In[Sc.Mate].bPass = true; Sc.Step();
		bool bCaught = false;
		for (int32 f = 0; f < 120 && !bCaught; ++f) { Sc.Clear(); Sc.Step(); bCaught = Sc.Sim->GetSkaters()[Sc.Me].PassesReceived > R0; }
		if (Arm == 1) TestTrue(TEXT("a pass to the mate behind the passer's body reaches him (clear_own_body)"), bCaught);
		else AddInfo(FString::Printf(TEXT("PASSPOWER clear_own_body false: the pass behind the body caught %d"), bCaught ? 1 : 0));
	}
	// The neutral values: pass_speed whatever the hold.
	{
		FJamGameConfig G2 = G;
		Neutral(G2);
		FPassStage Sc(*C, G2);
		Sc.Sim->SetSkater(Sc.Mate, FVector2D(-5.0, 0.0), FVector2D::ZeroVector, UE_DOUBLE_PI);
		Sc.GivePuck(FVector2D(-20.0, 0.0), FVector2D(1.0, 0.0));
		Sc.Pass(30, FVector2D(1.0, 0.0), [] {});
		TestEqual(TEXT("neutral: a 30-frame hold still leaves at pass_speed (the game before)"), Sc.Sim->GetPuck().Vel.Size(), 14.0, 0.2);
		TestFalse(TEXT("neutral: no pass in flight is marked"), Sc.Sim->GetPuck().PassFrames > 0);
	}
	return true;
}

// Lane A207 (the release's acceptance rows, steps 4 to 6 of HokejAcceptDrill.cpp): the drill's scenes replayed in the sim with the hand the release chain's
// driver gives (the stick on the drill's cursor target, the hold the act line asks), so each row is proven headless before the window runs it:
//   shoot_tap / shoot_half / shoot_full: from 10 m straight at the net, held 2 / 8 / 30 frames: a goal within 120 frames (the doubled shot crosses the line
//     between substeps at 0.22 m a substep: the goal must still count)
//   pass: a tap at the mate 8 m to the side: caught; pass_power: held 36 frames at a mate 28 m away on the line to the net: leaves at full power, caught
//   onetimer: the mate's pass from (Gx - 6, 6) to the human at (Gx - 10, -2): received, and a tap inside the window is a one-timer that scores
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamPassShotRows, "Jam.Core.PassShotRows", JamPassStagedTests::Flags)
bool FJamPassShotRows::RunTest(const FString& Parameters)
{
	using namespace JamPassStagedTests;
	TArray<FString> Errors;
	UJamCartridge* C = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")), Errors);
	if (!C) return false;
	const FJamGameConfig& G = C->Game;
	// The drill parks every other skater in the human's own corner (HokejAcceptDrill: "Parked skaters sit in the human's own corner").
	auto Park = [](FPassStage& Sc)
	{
		const int32 Team = Sc.Sim->GetSkaters()[Sc.Me].Team;
		const double Dir = Sc.Sim->AttackGoalX(Team) > 0 ? 1.0 : -1.0;
		int32 k = 0;
		for (const FJamSkaterState& S : Sc.Sim->GetSkaters()) if (S.Index != Sc.Me && S.Index != Sc.Mate) Sc.Sim->SetSkater(S.Index, FVector2D(-Dir * (30.0 + 2.0 * (k % 4)), -18.0 + 2.0 * (k++ / 4)), FVector2D::ZeroVector, 0.0);
	};
	auto GoalIn = [](FPassStage& Sc, int32 Frames) { const int32 S0 = Sc.Sim->GetScore(0) + Sc.Sim->GetScore(1); for (int32 f = 0; f < Frames; ++f) { Sc.Clear(); Sc.Step(); if (Sc.Sim->GetScore(0) + Sc.Sim->GetScore(1) > S0) return true; } return false; };
	// Step 4.
	{
		static const int32 Holds[3] = {2, 8, 30};
		static const TCHAR* Names[3] = {TEXT("shoot_tap"), TEXT("shoot_half"), TEXT("shoot_full")};
		for (int32 k = 0; k < 3; ++k)
		{
			FPassStage Sc(*C, G);
			Park(Sc);
			const int32 Team = Sc.Sim->GetSkaters()[Sc.Me].Team;
			const double Gx = Sc.Sim->AttackGoalX(Team), Dir = Gx > 0 ? 1.0 : -1.0;
			const FVector2D Fwd(Dir, 0.0);
			Sc.Sim->SetSkater(Sc.Me, FVector2D(Gx - Dir * 10.0, 0.0), FVector2D::ZeroVector, Dir > 0 ? 0.0 : UE_DOUBLE_PI);
			for (int32 i = 0; i < 2; ++i) { Sc.Sim->SetPuck(Sc.Sim->GetSkaters()[Sc.Me].BladePos, FVector2D::ZeroVector); Sc.Clear(); Sc.In[Sc.Me].Stick = JamAI::FVec2(Fwd.X, Fwd.Y); Sc.Step(); }
			for (int32 f = 0; f < Holds[k] + 1; ++f) { Sc.Clear(); Sc.In[Sc.Me].Stick = JamAI::FVec2(Fwd.X, Fwd.Y); Sc.In[Sc.Me].bShootHeld = f < Holds[k]; Sc.Step(); }
			const double V = Sc.Sim->GetPuck().Vel.Size();
			const bool bGoal = GoalIn(Sc, 120);
			AddInfo(FString::Printf(TEXT("PASSSHOTROW %s: release %.1f m/s, goal %d"), Names[k], V, bGoal ? 1 : 0));
			TestTrue(*FString::Printf(TEXT("%s: the shot from 10 m scores (%.1f m/s)"), Names[k], V), bGoal);
		}
	}
	// Step 5: the tap to the mate 8 m to the side, the full pass to the mate 28 m ahead.
	for (int32 k = 2; k <= 3; ++k)
	{
		FPassStage Sc(*C, G);
		Park(Sc);
		const int32 Team = Sc.Sim->GetSkaters()[Sc.Me].Team;
		const double Gx = Sc.Sim->AttackGoalX(Team), Dir = Gx > 0 ? 1.0 : -1.0;
		const double Face = Dir > 0 ? 0.0 : UE_DOUBLE_PI;
		const FVector2D HPos = k == 2 ? FVector2D(Gx - Dir * 16.0, -4.0) : FVector2D(Gx - Dir * 40.0, -4.0);
		const FVector2D MPos = k == 2 ? FVector2D(Gx - Dir * 16.0, 4.0) : FVector2D(Gx - Dir * 12.0, -4.0);
		Sc.Sim->SetSkater(Sc.Mate, MPos, FVector2D::ZeroVector, Face + UE_DOUBLE_PI);
		Sc.Sim->SetSkater(Sc.Me, HPos, FVector2D::ZeroVector, Face);
		const FVector2D ToM = (MPos - HPos).GetSafeNormal();
		for (int32 i = 0; i < 2; ++i) { Sc.Sim->SetPuck(Sc.Sim->GetSkaters()[Sc.Me].BladePos, FVector2D::ZeroVector); Sc.Clear(); Sc.In[Sc.Me].Stick = JamAI::FVec2(ToM.X, ToM.Y); Sc.Step(); }
		for (int32 i = 0; i < 40; ++i) { Sc.Clear(); Sc.In[Sc.Me].Stick = JamAI::FVec2(ToM.X, ToM.Y); Sc.Step(); }   // the drill's 45 frames of hand
		const int32 Hold = k == 2 ? 2 : G.Pass.PowerFullFrames + 6;
		Sc.Pass(Hold, ToM, [] {});
		const double V0 = Sc.Sim->GetPuck().Vel.Size();
		const FFlightOut O = Fly(Sc, 200, [] {});
		AddInfo(FString::Printf(TEXT("PASSSHOTROW %s: release %.1f m/s (full %.1f), caught %d after %d frames"), k == 2 ? TEXT("pass") : TEXT("pass_power"), V0, Sc.Sim->PassFullSpeed(), O.bCaught ? 1 : 0, O.Frames));
		if (k == 2) TestTrue(TEXT("pass: the tap reaches the mate 8 m away"), O.bCaught);
		else TestTrue(*FString::Printf(TEXT("pass_power: the full hold leaves at full power (%.1f) and the mate 28 m away takes it"), V0), O.bCaught && V0 >= Sc.Sim->PassFullSpeed() - 0.5);
	}
	// Step 6: the mate's pass to the human, his tap inside the window.
	{
		FPassStage Sc(*C, G);
		Park(Sc);
		const int32 Team = Sc.Sim->GetSkaters()[Sc.Me].Team;
		const double Gx = Sc.Sim->AttackGoalX(Team), Dir = Gx > 0 ? 1.0 : -1.0;
		const FVector2D HPos(Gx - Dir * 10.0, -2.0), MPos(Gx - Dir * 6.0, 6.0);
		const FVector2D ToNet = (FVector2D(Gx, 0.0) - HPos).GetSafeNormal();
		Sc.Sim->SetSkater(Sc.Me, HPos, FVector2D::ZeroVector, FMath::Atan2(ToNet.Y, ToNet.X));
		const FVector2D MToH = (HPos - MPos).GetSafeNormal();
		Sc.Sim->SetSkater(Sc.Mate, MPos, FVector2D::ZeroVector, FMath::Atan2(MToH.Y, MToH.X));
		for (int32 i = 0; i < 2; ++i) { Sc.Sim->SetPuck(Sc.Sim->GetSkaters()[Sc.Mate].BladePos, FVector2D::ZeroVector); Sc.Clear(); Sc.In[Sc.Me].Stick = JamAI::FVec2(ToNet.X, ToNet.Y); Sc.Step(); }
		const int32 R0 = Sc.Sim->GetSkaters()[Sc.Me].PassesReceived;
		Sc.Clear(); Sc.In[Sc.Mate].bPass = true; Sc.In[Sc.Me].Stick = JamAI::FVec2(ToNet.X, ToNet.Y); Sc.Step();
		bool bRecv = false, bOne = false, bGoal = false;
		const int32 S0 = Sc.Sim->GetScore(0) + Sc.Sim->GetScore(1);
		for (int32 f = 0; f < 200; ++f)
		{
			Sc.Clear(); Sc.In[Sc.Me].Stick = JamAI::FVec2(ToNet.X, ToNet.Y);
			if (bRecv && !bOne) Sc.In[Sc.Me].bShoot = true;
			Sc.Step();
			if (!bRecv && Sc.Sim->GetSkaters()[Sc.Me].PassesReceived > R0) bRecv = true;
			if (Sc.Sim->GetSkaters()[Sc.Me].bOneTimerThisFrame) bOne = true;
			if (Sc.Sim->GetScore(0) + Sc.Sim->GetScore(1) > S0) { bGoal = true; break; }
		}
		AddInfo(FString::Printf(TEXT("PASSSHOTROW onetimer: received %d, one-timer %d, goal %d"), bRecv ? 1 : 0, bOne ? 1 : 0, bGoal ? 1 : 0));
		TestTrue(TEXT("onetimer: the mate's pass is received and the tap scores or is a one-timer"), bRecv && (bGoal || bOne));
	}
	return true;
}

#endif
