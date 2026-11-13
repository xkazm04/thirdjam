// Lane A102 (track F2, carrier skill): the carry drill. One scene, used twice: headless by Jam.Core.Movement.StopAndGo (JamCarrierSkillTests.cpp) and in a
// package by jam.ctrl.Drill 21 (HokejGameMode.cpp), so the number the gate holds and the scene a person watches are the same code.
//
// The scene: the human carries along the attack at CarryMps; the opposing chaser stands DefenderM ahead, up to a metre off the line (the seed decides), and the
// real brain plays it from the moment the puck is seated (SettleFrames); everyone else is parked in a corner with no input. The scripted human uses the move
// keys and the mouse-style stick aim only, no button:
//   stop-and-go: at BrakeAtM of the defender it pulls back and to the open side (the reverse brake) until it is slower than GoBelowMps, cuts CutDeg off the line
//   to that side (the first-step burst), and goes straight again once it is a metre past; from the stop on, the cursor is on the far side of the body.
//   straight charge: W held, the blade ahead.
// BrakeAtM 9.5 is the clean stop against a pro: the brain does not leave its defender standing, it sends it at the carrier (11 m/s at contact), and a stop
// begun at the brief's 5 m is run over in the middle of the brake (0 of 200). MOVEMENT-2.md section 3 has the share by where the stop begins.
// A carry ends Past (2 m beyond the defender along the attack with the puck, and still with it half a second later), or by how the puck was lost.
// Header only; no state in the sim. docs/experiments/MOVEMENT-2.md.
#pragma once

#include "CoreMinimal.h"
#include "JamSim.h"
#include "JamEventBus.h"
#include "JamRandom.h"

struct FJamCarryDrill
{
	enum EEnd { Running = -1, Past = 0, Poke, Lift, Down, Stumble, Touch, Timeout, Setup, EndCount };
	static const TCHAR* EndName(int32 E)
	{
		static const TCHAR* Names[EndCount] = { TEXT("past"), TEXT("poke"), TEXT("lift"), TEXT("down"), TEXT("stumble"), TEXT("touch"), TEXT("timeout"), TEXT("setup") };
		return E >= 0 && E < EndCount ? Names[E] : TEXT("running");
	}
	static constexpr int32 SettleFrames = 10;   // the carry assist seats the puck; the defender stands
	static constexpr int32 MaxFrames = 370;

	// The script (what the player does).
	bool bStopAndGo = true;     // false = the straight charge
	double CarryMps = 9.0;      // the approach
	double DefenderM = 12.0;    // the standing defender's distance when the brain takes it over
	double BrakeAtM = 9.5;      // the gap at which the stop begins (the pro commits from 12 m; 9.5 m is a quarter of a second after it starts)
	double CutDeg = 60.0;       // the cut off the line of attack, to the side the defender is not on
	double GoBelowMps = 2.0;    // the stop is held until the carrier is slower than this
	bool bStickAway = true;     // from the stop on the cursor is on the far side of the body from the defender (at most StickDegPerFrame a frame)
	double StickDegPerFrame = 15.0;
	bool bBrake = true;         // false = the cut without the stop: at BrakeAtM the carrier cuts at once
	// Lane A121 (track F2, evolution 2): the late answer. On the first frame of the cut the carrier presses 1 = the spin (E: the body between for the spin's
	// frames) or 2 = the deke tap (the puck to the blade side, the sweep's shield). 0 = neither.
	int32 Answer = 0;
	// A person is not a metronome: the seed moves the approach speed (+-0.5 m/s), the defender's distance (+-1 m) and offset across the line (+-1.5 m), the
	// gap at which the stop begins (+-0.75 m), the cut's angle (+-10 degrees) and the frames before the go (0 to 4), each times Spread. 0 = the script exactly.
	double Spread = 1.0;

	// The scene and the run.
	int32 Me = -1, Def = -1;
	double Dir = 1.0;           // +1 = the human attacks +x
	int32 End = Running, Frames = 0, Phase = 0, PhaseFrames = 0, PastFrame = -1;
	int32 BrakeFrames = 0, DefLifts = 0, DefPokes = 0, DefChecks = 0, BodyShieldRefused = 0;
	double Side = 0.0, CarryMag = 1.0, MinGapM = 1e9;
	double LiveCarryMps = 9.0, LiveDefenderM = 12.0, LiveBrakeAtM = 9.5, LiveCutDeg = 60.0;   // the script's numbers with this seed's spread
	int32 ReactFrames = 0, GoWait = -1;
	FVector2D StickDir = FVector2D(1, 0);
	bool bPoked = false, bLifted = false, bDown = false, bBodyBefore = false;

	// Places the scene. False when the cartridge has no human or no opponent.
	bool Stage(UJamSim& Sim, uint64 Seed)
	{
		End = Running; Frames = Phase = PhaseFrames = 0; PastFrame = -1;
		BrakeFrames = DefLifts = DefPokes = DefChecks = BodyShieldRefused = 0;
		Side = 0.0; MinGapM = 1e9; GoWait = -1;
		bPoked = bLifted = bDown = bBodyBefore = false;
		Me = Sim.GetHumanIndex();
		Def = -1;
		const TArray<FJamSkaterState>& Sk = Sim.GetSkaters();
		if (!Sk.IsValidIndex(Me)) { End = Setup; return false; }
		const FName Chaser(TEXT("chaser"));
		for (const FJamSkaterState& S : Sk) if (S.Team != Sk[Me].Team && S.Role == Chaser) { Def = S.Index; break; }
		if (Def < 0) for (const FJamSkaterState& S : Sk) if (S.Team != Sk[Me].Team) { Def = S.Index; break; }
		if (Def < 0) { End = Setup; return false; }
		Dir = Sim.AttackGoalX(Sk[Me].Team) >= 0.0 ? 1.0 : -1.0;
		const int32 N = Sk.Num();
		for (int32 i = 0; i < N; ++i) Sim.SetSkater(i, FVector2D(Dir * (-40.0 + i * 1.5), 19.0), FVector2D::ZeroVector, 0);
		FJamRandom Rng(Seed * 7919ull + 17ull);
		const double U0 = Rng.Range(-1.0, 1.0), U1 = Rng.Range(-1.0, 1.0), U2 = Rng.Range(-1.0, 1.0), U3 = Rng.Range(-1.0, 1.0), U4 = Rng.Range(-1.0, 1.0);
		const int32 U5 = Rng.NextInt(5);
		const double Sp = FMath::Clamp(Spread, 0.0, 2.0);
		const double DefZ = U0 * (1.0 + 0.5 * Sp);
		LiveCarryMps = CarryMps + 0.5 * Sp * U1;
		LiveDefenderM = DefenderM + 1.0 * Sp * U2;
		LiveBrakeAtM = BrakeAtM + 0.75 * Sp * U3;
		LiveCutDeg = CutDeg + 10.0 * Sp * U4;
		ReactFrames = Sp > 0.0 ? FMath::RoundToInt32(U5 * FMath::Min(Sp, 1.0)) : 0;
		const double Top = Sim.GetEffectiveStat(EJamStat::SkaterSpeed, Me) * FMath::Clamp(Sim.GetConfig().Skater.CarrySpeedMult, 0.0, 1.0);
		CarryMag = FMath::Clamp(LiveCarryMps / FMath::Max(Top, 1e-6), 0.0, 1.0);
		const FVector2D Start(Dir * -13.5, 0.0), V(Dir * LiveCarryMps, 0.0);
		Sim.SetSkater(Me, Start, V, Dir > 0.0 ? 0.0 : UE_DOUBLE_PI);
		Sim.SetPuck(Start + FVector2D(Dir * 0.45, 0.0), V);   // against the body: a touch, then the carry assist seats it on the blade
		Sim.SetSkater(Def, Start + FVector2D(Dir * (LiveDefenderM + LiveCarryMps * SettleFrames / 60.0), DefZ), FVector2D::ZeroVector, Dir > 0.0 ? UE_DOUBLE_PI : 0.0);
		StickDir = FVector2D(Dir, 0.0);
		return true;
	}

	// Lane A121: the late answer inside 7 m (MOVEMENT-3.md section 5): no stop, the cut at AtM with the spin (InAnswer 1), the deke tap (2) or nothing (0).
	static FJamCarryDrill Late(int32 InAnswer, double AtM = 6.0)
	{
		FJamCarryDrill D;
		D.bBrake = false;
		D.BrakeAtM = AtM;
		D.Answer = InAnswer;
		return D;
	}

	bool IsLive() const { return End == Running && Me >= 0 && Def >= 0; }
	bool IsParked(int32 i) const { return i != Me && i != Def; }
	bool DefenderWaits() const { return Frames < SettleFrames; }

	// The scripted human's input for this frame (before the step). Advances the script.
	FJamInput HumanInput(const UJamSim& Sim)
	{
		FJamInput In;
		const FJamSkaterState& S = Sim.GetSkaters()[Me];
		const FJamSkaterState& D = Sim.GetSkaters()[Def];
		const double Gap = FVector2D::Distance(S.Pos, D.Pos);
		FVector2D Move(Dir * CarryMag, 0.0);
		FVector2D StickTo(Dir, 0.0);
		if (bStopAndGo && Frames >= SettleFrames)
		{
			const double CutRad = FMath::DegreesToRadians(LiveCutDeg);
			if (Phase == 0 && Gap <= LiveBrakeAtM) { Phase = bBrake ? 1 : 2; PhaseFrames = 0; Side = D.Pos.Y - S.Pos.Y >= 0.0 ? -1.0 : 1.0; }
			if (Phase == 1 && (S.Vel.Size() <= GoBelowMps || PhaseFrames >= 45))
			{
				// The go comes ReactFrames after the carrier is slow enough (the stop is held meanwhile).
				if (GoWait < 0) GoWait = ReactFrames;
				if (GoWait-- <= 0) { Phase = 2; PhaseFrames = 0; }
			}
			if (Phase == 2 && ((S.Pos.X - D.Pos.X) * Dir >= 1.0 || PhaseFrames >= 75)) { Phase = 3; PhaseFrames = 0; }
			if (Phase == 1) { Move = FVector2D(-Dir, Side).GetSafeNormal(); ++BrakeFrames; }   // S and the side key
			else if (Phase == 2) Move = FVector2D(Dir * FMath::Cos(CutRad), Side * FMath::Sin(CutRad));
			else if (Phase == 3) Move = FVector2D(Dir, 0.0);
			if (Phase == 2 && PhaseFrames == 0) { In.bSpin = Answer == 1; In.bDeke = Answer == 2; }   // lane A121: the late answer goes with the cut
			++PhaseFrames;
			if (bStickAway && Phase >= 1 && Gap > 1e-6) StickTo = (S.Pos - D.Pos) / Gap;
		}
		// The cursor moves at most StickDegPerFrame a frame, the short way round.
		const double A = FMath::Atan2(StickDir.Y, StickDir.X);
		double Turn = FMath::Atan2(StickTo.Y, StickTo.X) - A;
		while (Turn > UE_DOUBLE_PI) Turn -= 2.0 * UE_DOUBLE_PI;
		while (Turn < -UE_DOUBLE_PI) Turn += 2.0 * UE_DOUBLE_PI;
		const double MaxTurn = FMath::DegreesToRadians(StickDegPerFrame);
		const double NewA = A + FMath::Clamp(Turn, -MaxTurn, MaxTurn);
		StickDir = FVector2D(FMath::Cos(NewA), FMath::Sin(NewA));
		In.Move = Move;
		In.Stick = StickDir;
		return In;
	}

	// Before the step, with the defender's input of this frame: its presses, and whether the body is between right now.
	void BeforeStep(const UJamSim& Sim, const FJamInput& DefIn)
	{
		if (DefIn.bLift) ++DefLifts;
		if (DefIn.bPoke) ++DefPokes;
		if (DefIn.bCheck) ++DefChecks;
		MinGapM = FMath::Min(MinGapM, FVector2D::Distance(Sim.GetSkaters()[Me].Pos, Sim.GetSkaters()[Def].Pos));
		bBodyBefore = Sim.IsBodyShielded(Sim.GetSkaters()[Def], Sim.GetSkaters()[Me]);
	}

	// After the step: the frame's events and how the carry stands.
	void AfterStep(const UJamSim& Sim)
	{
		if (End != Running) return;
		++Frames;
		static const FName Clean(TEXT("clean"));
		for (const FJamSimEvent& E : Sim.GetFrameEvents())
		{
			if (E.Name == JamEvents::Poke && E.Who == Def && E.Kind == Clean && E.Strength > 0.5f) bPoked = true;
			if (E.Name == JamEvents::Lift && E.Who == Def && E.Strength > 0.5f) bLifted = true;
			if (E.Name == JamEvents::Lift && E.Who == Def && E.Strength < 0.5f && bBodyBefore) ++BodyShieldRefused;
			if (E.Name == JamEvents::Knockdown && E.Who == Me) bDown = true;
		}
		if (Frames < SettleFrames) return;
		const FJamSkaterState& S = Sim.GetSkaters()[Me];
		const FJamSkaterState& D = Sim.GetSkaters()[Def];
		if (Frames == SettleFrames && Sim.CarrierIndex() != Me) { End = Setup; return; }
		if (Sim.GetPuck().LastTouch != Me || S.IsDown())
		{
			End = bPoked ? Poke : bLifted ? Lift : (bDown || S.IsDown()) ? Down : S.IsStumbling() ? Stumble : Touch;
			return;
		}
		if (PastFrame < 0 && (S.Pos.X - D.Pos.X) * Dir >= 2.0 && Sim.CarrierIndex() == Me) PastFrame = Frames;
		if (PastFrame >= 0 && Frames - PastFrame >= 30) { End = Sim.CarrierIndex() == Me ? Past : (S.IsStumbling() ? Stumble : Touch); return; }
		if (Frames >= MaxFrames) End = Timeout;
	}
};
