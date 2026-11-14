// Lane A166 (track F2, evolution 7, docs/experiments/MOVEMENT-8.md): the two warnings. Pure read-outs of the sim: nothing here writes to it, and nothing
// in the sim reads this file. One rule each, shared by the HUD (UJamHudWidget::UpdateMarks draws the marks), the trace (Bar.Movement.CarryTrace measures
// them against what happened) and the instrument (the scripted human may brake for the first: env JAM_PLAYTESTER_CONTEST=brake, off by default).
//   The contested puck: nobody carries, the human skates at the puck and will have it inside a second, and an opponent skates at it and at him and will
//     be there about when he is. MOVEMENT-7 section 4: 65 % of the human's knockdowns at pro come within half a second of his first touch, at 16 m/s of
//     closing: he wins the race and the pressed check lands eight frames later.
//   The stop's distance: the human carries at speed and an opponent ahead of his travel (within 60 degrees of it) closes on him. The mark stands five stick lengths (8 m) from him
//     towards that opponent; it is armed while the opponent is up to 20 m off and lit from a quarter second before he is 11 m off until he is 7 m off: the window in which law 3's stop passes
//     (REVIEW-2: 100 % at 8 m, 97 % at 9.5 m, 83.5 % at 11 m, 66 % at 7 m). Inside 7 m it is gone: a stop begun there loses the feet.
#pragma once

#include "CoreMinimal.h"
#include "JamSim.h"

namespace JamCarrierMarks
{
	// The stick: a skater has "reached" a puck this far from its body.
	constexpr double ReachM = 1.6;

	struct FContestRule
	{
		double HumanSecMax = 1.0;      // the human has the puck inside this long
		double TowardMps = 3.0;        // the puck and each of the two close at this pace or more
		double OppEarlySec = 0.25;     // the opponent is there no more than this before him (earlier: it is the opponent's puck and the human is the hitter)
		double OppLateSec = 0.6;       // ... and no more than this after him
		double ClosingMps = 8.0;       // the two close on each other at this pace or more (head-on, not side by side)
	};
	struct FContested
	{
		bool bOn = false;
		int32 Opp = -1;
		double HumanSec = 0.0, OppSec = 0.0, ClosingMps = 0.0;
	};

	// Seconds until S's stick reaches the puck at P moving at PV, at the pace the two close at now (a pass coming at a standing skater closes as fast as
	// a skater going at a lying puck); 1e9 when they do not close. OutToward: that pace (m/s).
	inline double SecondsTo(const FJamSkaterState& S, const FVector2D& P, const FVector2D& PV, double& OutToward)
	{
		const FVector2D To = P - S.Pos;
		const double D = To.Size();
		OutToward = D > 1e-6 ? FVector2D::DotProduct(S.Vel - PV, To / D) : 0.0;
		const double Gap = D - ReachM;
		if (Gap <= 0.0) return 0.0;
		return OutToward > 0.5 ? Gap / OutToward : 1e9;
	}

	inline FContested Contested(const UJamSim& Sim, int32 Me, const FContestRule& K = FContestRule())
	{
		FContested R;
		const TArray<FJamSkaterState>& Sk = Sim.GetSkaters();
		if (!Sk.IsValidIndex(Me) || Sim.GetPhase() != EJamSimPhase::Playing || Sim.IsDrawOpen() || Sim.CarrierIndex() >= 0) return R;
		const FJamSkaterState& H = Sk[Me];
		if (!H.CanAct()) return R;
		const FVector2D P = Sim.GetPuck().Pos, PV = Sim.GetPuck().Vel;
		double HToward = 0.0;
		const double Th = SecondsTo(H, P, PV, HToward);
		if (HToward < K.TowardMps || Th > K.HumanSecMax) return R;
		for (const FJamSkaterState& O : Sk)
		{
			if (O.Team == H.Team || !O.CanAct()) continue;
			double OToward = 0.0;
			const double To = SecondsTo(O, P, PV, OToward);
			if (OToward < K.TowardMps || To < Th - K.OppEarlySec || To > Th + K.OppLateSec) continue;
			const FVector2D Between = O.Pos - H.Pos;
			const double D = Between.Size();
			const double Closing = D > 1e-6 ? FVector2D::DotProduct(H.Vel - O.Vel, Between / D) : 0.0;
			if (Closing < K.ClosingMps) continue;
			if (!R.bOn || To < R.OppSec) { R.bOn = true; R.Opp = O.Index; R.HumanSec = Th; R.OppSec = To; R.ClosingMps = Closing; }
		}
		return R;
	}

	struct FStopRule
	{
		double AheadM = 8.0;           // where the mark stands: five stick lengths from the human, towards the opponent
		double LitNearM = 7.0;         // lit while the opponent is this far off or more ...
		double LitFarM = 11.0;         // ... and this far off or less, plus the hand's lead:
		double ReactSec = 0.25;        // the mark lights this long before the opponent is at LitFarM at the pace the two close at, so a stop pressed a quarter
		                               // second after it lights begins inside the window (head-on at 15 m/s the window itself lasts a quarter second)
		double ArmFarM = 20.0;         // armed (shown, not lit) from here in
		double ClosingMps = 3.0;       // the two close at this pace or more
		double SpeedMps = 3.0;         // the human moves at this pace or more (a stop is a move only for one who moves)
		double ConeCos = 0.5;          // the opponent is within 60 degrees of the human's travel (35 degrees finds 59 % of the knockdowns it is for, 60 degrees 72 %, at the same precision)
	};
	struct FStopMark
	{
		bool bShow = false, bLit = false;
		int32 Def = -1;
		double DistM = 0.0, ClosingMps = 0.0;
		FVector2D At = FVector2D::ZeroVector;   // the mark's place on the ice (rink metres)
	};

	inline FStopMark StopMark(const UJamSim& Sim, int32 Me, const FStopRule& K = FStopRule())
	{
		FStopMark R;
		const TArray<FJamSkaterState>& Sk = Sim.GetSkaters();
		if (!Sk.IsValidIndex(Me) || Sim.GetPhase() != EJamSimPhase::Playing || Sim.CarrierIndex() != Me) return R;
		const FJamSkaterState& H = Sk[Me];
		const double Speed = H.Vel.Size();
		if (!H.CanAct() || H.bReverseBraking || Speed < K.SpeedMps) return R;   // braking already: the mark has said what it had to say
		const FVector2D Head = H.Vel / Speed;
		for (const FJamSkaterState& O : Sk)
		{
			if (O.Team == H.Team || !O.CanAct()) continue;
			const FVector2D To = O.Pos - H.Pos;
			const double D = To.Size();
			if (D < K.LitNearM || D > K.ArmFarM) continue;
			const FVector2D N = To / D;
			if (FVector2D::DotProduct(N, Head) < K.ConeCos) continue;
			const double Closing = FVector2D::DotProduct(H.Vel - O.Vel, N);
			if (Closing < K.ClosingMps) continue;
			if (!R.bShow || D < R.DistM) { R.bShow = true; R.Def = O.Index; R.DistM = D; R.ClosingMps = Closing; R.At = H.Pos + N * K.AheadM; }
		}
		R.bLit = R.bShow && R.DistM <= K.LitFarM + FMath::Clamp(R.ClosingMps, 0.0, 25.0) * K.ReactSec;
		return R;
	}
}
