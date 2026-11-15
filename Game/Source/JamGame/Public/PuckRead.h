// Lane A210 (track F3, wave 4, R-0067 point 2 and R-0068: "Puck does not stick with players"): the puck is SEEN to belong to someone.
// Presentation only: everything here reads the sim after a step and writes nothing back (the default match's hash is untouched).
//   THE CARRIER THE SCREEN SHOWS (jam.puck.Show 1): one answer per sim frame, taken from the sim's carrier of record AFTER the step
//     (UJamSim::CarrierIndex), for both things a player reads possession from: the figure's SetHasPuck flag and the place the puck is drawn. Before
//     (jam.puck.Show 0, package H34) the flag came from the skater state's bHasPuck, which the sim sets in the middle of its step, and the drawn place
//     from CarrierIndex after it: device B measured the two apart on 14 % of the human's and 54 to 82 % of the bots' flagged frames (status/E3-report.md).
//     The sim's carrier of record drops out for single frames while a man still has the puck (the puck a few cm past his reach for a frame): the shown
//     carrier is kept for at most jam.puck.Keep frames (default 6) while the puck is still his last touch, he can act, he did not shoot or pass, and
//     the puck is within KeepReachK of his reach; never while he is down or hit.
//   THE DRAWN PLACE: on the shown carrier's puck_socket (device B's blade), else where the sim has it. A change of hands does not pop: the offset between
//     where it was drawn and where it now belongs runs out over BlendInFrames (a pickup) or BlendOutFrames (a release).
//   THE PLAY CALLS (device B's H55.6, by name, exists-checked): PlayReceive() on the frame a mate's pass is seated; SetPassCharge(0..1) while a pass is
//     held and 0 once on its end; PlayCallForPass() by the mate the human's pass would go to now (the sim's own HumanPassTarget), at most every
//     CallEveryFrames; SetLookAtPuck(false) once on the human's figure; the puck trail's User.Carried.
//   THROUGH THE FRAME'S ANIMATION (jam.puck.Follow 1): the game mode places the puck before the figures' poses of the frame are evaluated; once on the
//     blade the puck ticks after the animation and takes the socket's place again (AJamPuck::SetFollow), so a moving stick does not leave it behind.
//   PROOF (jam.puck.Probe N): every N sim frames one JAMPUCK show line: of the sim's carry frames, how many drew the puck within OnSocketCm of the
//     carrier's socket, for the human and for the bots; flagged frames that disagree with the sim's carrier.
#pragma once

#include "CoreMinimal.h"
#include "JamSim.h"

class AJamSkater;
class AJamPuck;

struct JAMGAME_API FJamPuckShow
{
	static constexpr double KeepReachK = 1.35;      // the kept carrier's puck may sit this share of his reach away
	static constexpr int32 BlendInFrames = 2;       // a pickup: the drawn puck travels to the blade over this many sim frames
	static constexpr int32 BlendOutFrames = 0;      // a release (a shot, a pass, a loss): at the sim's place at once: a loose puck is never drawn off the sim's (2 until 04:30 on 15 Nov: a goal the frame after the shot stopped the sim and left the disc half a metre from the HUD's ring in the net)
	static constexpr double GoalDropSeconds = 0.17;   // after a goal the sim's puck stays at the height it crossed the line at; the drawn one falls to the ice in this time
	static constexpr int32 ReleaseNoKeepFrames = 30;   // no bridge over a drop-out for this long after his shot or pass flag (the flag can come a frame or more before the carrier of record falls away)
	static constexpr double OnSocketCm = 10.0;      // the probe's "on the blade"
	static constexpr double SideBandCm = 8.0;       // the sim's puck changes side for the carry floor only once it is this far over his line
	// The Director's ruling of 02:10: the DRAWN puck no more than jam.puck.MaxOff cm (45) from the sim's: on the way from the sim's puck to the blade's
	// socket it stops there. Pure.
	static FVector Capped(const FVector& SimCm, const FVector& SocketCm, double MaxOffCm);
	// The side for the carry floor from the puck's place across his line (LocalYCm: + his right), keeping Prev inside the band. Pure.
	static int32 SideFor(double LocalYCm, int32 Prev) { return LocalYCm > SideBandCm ? -1 : LocalYCm < -SideBandCm ? 1 : Prev; }
	static constexpr int32 ReceiveEveryFrames = 20; // PlayReceive: once a reception (a deflection and the seat that follows are one catch to the eye)
	static constexpr int32 CallEveryFrames = 30;    // PlayCallForPass: at most every 0.5 s ...
	static constexpr int32 CallSameMateFrames = 150;   // ... and the same mate again only after 2.5 s

	// The carrier the screen shows after a sim step. Prev: last frame's shown carrier (-1 nobody); SimCarrier: UJamSim::CarrierIndex after the step;
	// the rest describe Prev: the puck's last touch is his, he can act, he shot or passed this frame, the puck's distance from his body and his reach.
	// InOutKept: frames Prev has been kept without the sim naming him (0 when the sim names a carrier). KeepFrames 0: the sim's carrier, nothing kept. Pure.
	static int32 ShownCarrier(int32 Prev, int32 SimCarrier, int32 LastTouch, bool bPrevCanAct, bool bPrevReleased, double PrevDistM, double PrevReachM, int32 KeepFrames, int32& InOutKept);
	// A change from one stick straight to another is shown once the sim has named the new man SettleFrames frames running (Wanted: ShownCarrier's answer;
	// bPrevHolds: the man before can act and did not release it; bNewPlays: the new man released it this frame, or is the human holding a shot or a
	// pass). From a loose puck (Prev -1): after SettleLooseFrames. Until then: Prev. Pure.
	static int32 Settled(int32 Prev, int32 Wanted, bool bPrevHolds, bool bNewPlays, int32 SettleFrames, int32 SettleLooseFrames, int32& InOutPending, int32& InOutPendingFrames);
	// The drawn place: Target (the blade's socket when carried, the sim's place otherwise) plus what is left of the offset it had at the change of
	// hands, SinceChange sim frames ago, eased out over BlendFrames. Pure.
	static FVector Drawn(const FVector& TargetCm, const FVector& OffsetAtChangeCm, float SinceChange, int32 BlendFrames);
	// The drawn height over the ice during the goal pause: AboveCm when it began, 0 after GoalDropSeconds, a fall (slow, then fast). Pure.
	static double GoalDropCm(double AboveCm, double SinceS) { const double U = FMath::Clamp(SinceS / GoalDropSeconds, 0.0, 1.0); return FMath::Max(0.0, AboveCm) * (1.0 - U * U); }
	// SetPassCharge's value for a hold of ChargeFrames with the cartridge's full_frames. Pure.
	static float PassCharge01(int32 ChargeFrames, int32 FullFrames) { return ChargeFrames <= 0 ? 0.f : FMath::Clamp(static_cast<float>(ChargeFrames) / static_cast<float>(FMath::Max(FullFrames, 1)), 0.f, 1.f); }
	// May a mate call for the pass now? Pure.
	static bool MayCall(int64 Frame, int64 LastCallFrame, int32 Mate, int32 LastMate) { return Mate >= 0 && Frame - LastCallFrame >= (Mate == LastMate ? CallSameMateFrames : CallEveryFrames); }

	// The game mode's two calls. StepSim: once per sim step, after the sim stepped and BEFORE the pawns' ApplySimFrame (it tells each pawn whether it is
	// the shown carrier). Render: each render frame after the skaters' ApplyRender; it places the puck (the game mode's old block).
	void StepSim(const UJamSim* Sim, const TArray<TObjectPtr<AJamSkater>>& Skaters, AJamPuck* Puck);
	void Render(const UJamSim* Sim, const TArray<TObjectPtr<AJamSkater>>& Skaters, AJamPuck* Puck, float Alpha);
	void Reset();

	static bool ShowOn();
	int32 GetShown() const { return Shown; }

private:
	int32 Shown = -1;
	int32 Kept = 0;
	int32 Pending = -1, PendingFrames = 0;   // a stick-to-stick change not yet shown, and the frames the sim has named him
	int32 PuckSide = 0;   // the side of the shown carrier's line the sim's puck is on (-1 his right, +1 his left), with a band of SideBandCm
	int64 LastFrame = -1;
	// the drawn place
	int32 DrawnCarrier = -1;
	double ChangeFrameF = -1e9;
	int32 ChangeBlend = BlendInFrames;
	FVector OffsetAtChange = FVector::ZeroVector;
	FVector LastDrawn = FVector::ZeroVector;
	bool bWasStopped = false;       // the sim was in its goal pause (or over) at the last drawn frame
	double StoppedAtS = 0.0;        // world seconds when that began: the drawn puck drops to the ice over GoalDropSeconds from then
	bool bEverDrawn = false;
	// the play calls
	TArray<int32> PrevReceived;
	TArray<int64> LastReceiveFrame;
	TArray<int64> LastReleaseFrame;   // per skater: the last sim frame his shot or pass flag was up
	TArray<bool> ChargeSent;
	TWeakObjectPtr<UObject> LookFigure;
	int64 LastCallFrame = -100000;
	int32 LastCallMate = -1;
	int32 TrailCarried = -1;
	// the probe (per sim frame, judged at the first render after the step)
	int64 ProbedFrame = -1;
	int32 ProbeFrames = 0;
	int32 CarryN[2] = { 0, 0 }, OnSocketN[2] = { 0, 0 };          // [0] the human, [1] the bots: the sim's carry frames, and those drawn on the socket
	int32 FlagN[2] = { 0, 0 }, FlagNotCarrierN[2] = { 0, 0 };     // frames a figure was told it has the puck, and those where the sim's carrier is not him
	int32 KeptN = 0, SettleN = 0, CallsN = 0, ReceivesN = 0;
	double WorstCm[2] = { 0.0, 0.0 };
	int32 LagN = 0, LagOver1N = 0, LagOver10N = 0;
	int32 BlendN[2] = { 0, 0 };
	double OffBladeSumCm[2] = { 0.0, 0.0 };
	double ToSimSumCm[2] = { 0.0, 0.0 }, ToSimMaxCm[2] = { 0.0, 0.0 };   // the drawn puck against the sim's, on carry frames
	double LagSumCm = 0.0, LagMaxCm = 0.0;
	int32 LooseDrawsN = 0, BridgedDrawsN = 0;   // the probe: drawn frames with no carrier of record: nobody shown / a man still shown (the bridge)
	double LooseMaxCm = 0.0, BridgedMaxCm = 0.0;   // ... and the drawn puck against the sim's on them
	// the strips (jam.puck.Capture)
	void Capture(const UJamSim* Sim);
	int64 CapFrom = -1, CapUntil = -1;
	FString CapKind;
	TMap<FString, int32> CapPerKind;
	bool bCapCarry = false, bCapPassHold = false, bCapShotHold = false;
	int32 CapReceived = 0;
};
