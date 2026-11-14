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
		double ClosingMps = 12.0;      // the two close on each other at this pace or more (head-on, not side by side). Director, 14 Nov, from REVIEW-3's
		                               // sweep by JAM_MARK_RULE (closing=12;stopclosing=8): it was 8, and the ring showed 18.6 / 19.3 / 29.4 times a match
		                               // (rookie / pro / all_star) for 2.85 knockdowns at rookie, right 15.6 % of the time there; at 12: 11.4 / 13.4 / 22.6,
		                               // recall down 6 to 8 points.
		int32 AfterDropFrames = 0;     // 0 = off (the Director, 06:45: lane A171 measured the quiet second: at rookie it takes 0.3 rings a match, at pro it takes 206 of the 316
		                               // knockdowns the ring precedes: he is flattened as he wins the draw. The rule stays, off.) Was 60: REVIEW-3 item 1: no ring for this long after a faceoff's drop: it lit in the second after
		                               // most draws the human won (two centres head-on at the puck is every draw, not a warning). 0 = as before.
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
		if (K.AfterDropFrames > 0 && Sim.GetDropFrame() >= 0 && Sim.GetFrame() - Sim.GetDropFrame() < K.AfterDropFrames) return R;
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

	// Lane A178 (track F2, evolution 8, docs/experiments/MOVEMENT-9.md): what is SHOWN is not the rule frame by frame. A real window (package H30) read the
	// contested ring's median time on screen as 5 frames, 8 showings of one frame: nobody saw it. A showing, once begun, stands MinFrames at least, and
	// stands through a gap of the rule of up to LingerFrames (one episode, one showing: no flicker). The HUD and the trace step the same state.
	struct FShowRule
	{
		int32 MinFrames = 24;          // the least time on screen (the screen lane's number: 0.4 s)
		int32 LingerFrames = 12;       // the rule may be off this long inside one showing
	};
	struct FMarkShow
	{
		int64 Since = -1, LastOn = -1, Until = -1;
		int32 Opp = -1;                // the opponent of the rule's last yes
		int32 Showings = 0;
		bool Shown(int64 Frame) const { return Since >= 0 && Frame >= Since && Frame <= Until; }
	};
	// One step a sim frame. bOn: the rule's answer this frame. Returns whether the mark is shown on Frame.
	inline bool StepShow(FMarkShow& S, bool bOn, int32 Opp, int64 Frame, const FShowRule& K = FShowRule())
	{
		if (S.Since >= 0 && Frame < S.LastOn) S = FMarkShow();   // the clock went back: a restart
		if (bOn)
		{
			if (!(S.Since >= 0 && Frame >= S.Since && Frame <= S.Until + 1)) { S.Since = Frame; ++S.Showings; }   // the frame after its last is still the same showing
			S.LastOn = Frame;
			S.Opp = Opp;
			S.Until = FMath::Max(S.Since + FMath::Max(1, K.MinFrames) - 1, Frame + FMath::Max(0, K.LingerFrames));
		}
		return S.Shown(Frame);
	}

	struct FStopRule
	{
		double AheadM = 8.5;           // where the mark stands: five stick lengths from the human, towards the opponent. Lane A194: 8 -> 8.5 m, the stop window's centre
		                               // since the free skaters are 15 % faster (pro: 8 m 94 %, 8.5 m 99.5 %, 9 m 94.5 %) and drill 21's stop distance; a drawing only
		double LitNearM = 7.0;         // lit while the opponent is this far off or more ...
		double LitFarM = 9.0;          // ... and this far off or less, plus the hand's lead. Lane A185 (track F2 evolution 9, MOVEMENT-10.md): 9.5 -> 9 m. With the free skaters
		                               // 15 % faster (skater.free_speed_mult) the defender closes sooner: the stop begun at 9.5 m passes a pro 78 % (97 before), at 9 m 94.5 %,
		                               // at 8.5 m 99.5 %, at 8 m 94 %, at 7 m 38 % (Bar.Curve.Law3, 200 seeds); all_star 96 / 99.5 / 100 / 99.5 % at 9.5 / 9 / 8.5 / 8 m.
		                               // Before that: Director, 14 Nov, from REVIEW-3's sweep of the stop by distance (200 seeds a row):
		                               // it was 11 m, and a stop begun at 11 / 12 / 13.5 m passes 83.5 / 56.5 / 52 % at pro and 53 / 28.5 / 19.5 % at all_star: the mark lit where
		                               // obeying it lost the feet. Begun at 9.5 m it passes 100 / 97 / 92 % (rookie / pro / all_star), at 8 m 100 % at every level. The mark's
		                               // recall of the knockdowns it is for: 76.7 -> 73.8 % at pro, lit frames down a fifth.
		double ReactSec = 0.25;        // the mark lights this long before the opponent is at LitFarM at the pace the two close at, so a stop pressed a quarter
		                               // second after it lights begins inside the window (head-on at 15 m/s the window itself lasts a quarter second)
		double ArmFarM = 20.0;         // armed (shown, not lit) from here in
		double ArmClosingMps = 2.0;    // lane A178: ... while the two close at this pace or more. The lit state keeps ClosingMps (8). Before, one number armed and
		                               // lit: in the window's drill the pale bar came up at 13.4 m and turned at 11.8 m, 3 to 10 frames later: nobody saw chalk
		                               // turn to colour. Armed from the moment he carries at an opponent ahead, the pale bar is there for the whole approach.
		double ClosingMps = 8.0;       // the two close at this pace or more (Director, 14 Nov, REVIEW-3's stopclosing=8: it was 3; see FContestRule::ClosingMps)
		double SpeedMps = 3.0;         // the human moves at this pace or more (a stop is a move only for one who moves)
		double DefTowardMps = 0.0;     // lane A178: to LIGHT, the opponent himself skates at the human at this pace or more (0 = no such condition: package H30, where
		                               // the human's own 8 m/s at a standing opponent lights the line). A measurement's number (JAM_MARK_RULE deftoward=): see MOVEMENT-9.
		double ConeCos = 0.5;          // the opponent is within 60 degrees of the human's travel (35 degrees finds 59 % of the knockdowns it is for, 60 degrees 72 %, at the same precision)
	};
	struct FStopMark
	{
		bool bShow = false, bLit = false;
		int32 Def = -1;
		double DistM = 0.0, ClosingMps = 0.0;
		FVector2D At = FVector2D::ZeroVector;   // the mark's place on the ice (rink metres): AheadM from him towards the opponent
		// Lane A178: his travel (unit), the mark's place straight ahead on it (AheadM), and how far off his travel the opponent is (degrees).
		FVector2D Head = FVector2D(1.0, 0.0), AtPath = FVector2D::ZeroVector;
		double OffDeg = 0.0;
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
			if (Closing < FMath::Min(K.ArmClosingMps, K.ClosingMps)) continue;
			// Lane A178: the opponent the stop is on for wins over a nearer one that merely stands ahead; among equals the nearer.
			const bool bLitNow = Closing >= K.ClosingMps && D <= K.LitFarM + FMath::Clamp(Closing, 0.0, 25.0) * K.ReactSec
				&& (K.DefTowardMps <= 0.0 || -FVector2D::DotProduct(O.Vel, N) >= K.DefTowardMps);
			if (!R.bShow || (bLitNow && !R.bLit) || (bLitNow == R.bLit && D < R.DistM))
			{
				R.bShow = true; R.bLit = bLitNow; R.Def = O.Index; R.DistM = D; R.ClosingMps = Closing; R.At = H.Pos + N * K.AheadM;
				R.OffDeg = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector2D::DotProduct(N, Head), -1.0, 1.0)));
			}
		}
		R.Head = Head;
		R.AtPath = H.Pos + Head * K.AheadM;
		return R;
	}
}
