// Lane A196 (track F3, evolution 9, R-0064 point 5, docs/experiments/HITS.md): "a hit is read by an eye that was looking elsewhere".
// Presentation only: everything here reads the sim's state and events after a step and writes nothing back (the default match's hash is untouched).
//   BEFORE (jam.hit.Tell): a pressed body check shows a chevron of light on the ice 1.4 to 2.4 m ahead of the checker (outside the HUD's rings): it slides out over the wind-up (loading),
//     is bright through the check window (the lunge), and goes out grey when the window runs out on nobody (a miss reads as a miss: the lunge, no burst).
//     A pressed cross-check shows a streak across the stick (chest height) through its wind-up, thrust forward on the shove's frame. A held cross-check
//     (bCrossBraced) holds the streak across the chest and an arc on the ice in front, brighter with speed.
//     jam.hit.Lean: while the figure lacks device B's PlayCheckWindup / SetCrossCheckHold, the drawn figure (the skater actor, never the sim) leans:
//     back over the wind-up, forward through the window, back to upright over 8 frames. B's takes, when the figure declares them, stand it down.
//   AT CONTACT (jam.hit.Burst): at the contact point a ring on the ice that grows to BurstRadiusM(closing speed) and a fan of shards out from the hit,
//     away from the victim's fall (nothing over a downed figure); a braced hit has a second ring and more shards. One JAMHIT line a hit and a miss.
//     The camera kick (CameraFollow, jam.hitkick) and the hit-stop (jam.hitstop) already answer the human's hits; the sound is the binder's MS_check.
//   AFTER (jam.hit.Down): a downed skater lies in a ring of 12 bars on the ice round his body that go out one by one as his down frames run out
//     (a countdown an eye reads; on the ice only: nothing over the figure). (jam.hit.PuckFlash): a puck stripped by a stick lift flashes a ring.
//   PROOF (jam.hit.Capture N): a still every sim frame from each of the human's presses to 30 frames after its contact or miss, N strips a kind.
// Lane A210 (the night of 14 Nov, R-0067, docs/experiments/HITS-2.md): "crosscheck has bug in animation displaying technical blue rectangle instead of
// animating player with stick into its position". The rectangle was this file's own bar across the chest of a held cross-check (and the bar across the
// stick of a pressed one). What ships now: THE FIGURE DOES THE HITTING, the marks are quiet helpers.
//   jam.hit.BraceBar 0, jam.hit.Streak 0: the two bars over the figure are not drawn (1 = package H34).
//   jam.hit.BTakes 2: device B's wind-up takes on every figure, the human's too; A's stand-in lean stands down wherever B's take plays.
//   jam.hit.Quiet 1: the chevron and the brace's arc 7 cm wide in the ice's white; the contact ring 0.95 to 1.35 m (jam.hit.RingScale), white, 8 frames,
//     no second ring on a braced hit (0 = package H34: 18 cm in the team's colour, a ring to 3 m for 14 frames). The downed man's oval is unchanged.
//   jam.hit.RingFar 1: the small ring is for a hit near the eye; from 10 m to 30 m from the camera it grows back to package H34's radius and 14 frames
//     (still white): a hit across the rink is 30 px of bodies and the ring says where.
//   jam.hit.Pop 1: the hit's coil and drive on the drawn figure (its scale, never the sim): sunk through the wind-up, sprung at the window, settled in 8.
//   jam.hit.BraceSet 1, jam.hit.BraceLean 12: the braced figure sets (a tenth lower, 6 % wider, pitched 12 degrees) while device B's hold cannot be
//     seen from behind: a bridge on the figure, to be switched off when B's pose reads.
//   jam.hit.Retime 1: a check that lands inside its wind-up (lane A209's rule) sends the checker's take to its contact pose (PlayCheckWindup(1)).
//   jam.hit.BraceArc: the arc's own switch. jam.hit.CaptureBots / CaptureBotsM / CaptureBotsMinM, -JamHitShots=<folder>, JAMHIT stick / step lines: the strips' tools.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blueprint/UserWidget.h"
#include "JamSim.h"
#include "HitRead.generated.h"

class AJamSkater;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;

// One bar of light: a box CentreCm (world), turned YawDeg about Z then PitchDeg, SizeCm (along, across, tall), in Colour at Emissive.
struct FJamHitBar
{
	FVector CentreCm = FVector::ZeroVector;
	float YawDeg = 0.f;
	float PitchDeg = 0.f;
	FVector SizeCm = FVector(10.0);
	FLinearColor Colour = FLinearColor::White;
	float Emissive = 1.f;
	uint8 Layer = 0;   // EJamHitLayer: which element drew it (tests and the probe)
};

enum class EJamHitLayer : uint8 { Tell = 1, Streak = 2, Brace = 3, Burst = 4, Shard = 5, Down = 6, PuckFlash = 7 };

// What a press came to (the JAMHIT line's result).
enum class EJamHitResult : uint8 { None, Bump, Stumble, Down, Miss, Strip };

// The pure part: fed every sim frame (skaters' state after the step, the frame's events), it keeps the presses, the hits and the downs; Draw turns them
// into bars for a render frame. No world, no UObject: the tests run it on hand-made states.
class JAMGAME_API FJamHitReadModel
{
public:
	static constexpr int32 MissFadeFrames = 12;
	static constexpr int32 BurstFrames = 14;
	static constexpr int32 FlashFrames = 18;
	static constexpr int32 LeanBackFrames = 8;
	static constexpr int32 DownSegments = 12;
	static constexpr double DownRingRadiusM = 1.5;    // the oval round a lying figure: along his body (centred on its middle)
	static constexpr double DownRingSideM = 0.95;     // and across it
	static constexpr double DownBodyM = 1.7;          // a lying figure's length from the skates along the fall
	static constexpr double DownBodyHalfWideM = 0.35;
	static constexpr double TellNearM = 1.4;    // the chevron's tip: from here at the press (outside the HUD's YOU and carrier rings, ~1 m) ...
	static constexpr double TellFarM = 2.4;     // ... to here when the wind-up is done
	static constexpr double BraceRadiusM = 1.35;
	static constexpr double RingNearM = 10.0;       // lane A210: the small ring within 10 m of the eye ...
	static constexpr double RingFarM = 30.0;        // ... the full one from 30 m
	static constexpr double RingMinM = 0.95;         // lane A210: the small contact ring never closer than this to the contact point (it clears the two bodies)

	// The burst's ring radius for a hit's closing speed (m/s): 0.8 m at a soft touch to 3.0 m at 14 m/s and over; a braced hit x 1.2.
	static double BurstRadiusM(double ClosingMps, bool bBraced);
	// Evolution 10 (the Director: "the ring is the loudest thing on the sheet"): the burst's weight 0..1 by the closing speed (closing / 14 m/s; a bump 0).
	// The ring keeps its radius by speed; its bars' width, brightness and count follow the weight and fall from the first frame: a light hit is about a
	// third of evolution 9's (width at weight 0.1: 8 cm against 31). No bar is drawn within BodyClearM of either body (standing: a circle round the
	// drawn figure; down: round the lying body).
	static float BurstWeight(double ClosingMps) { return static_cast<float>(FMath::Clamp(ClosingMps / 14.0, 0.0, 1.0)); }
	static float BurstBarWidthCm(float Weight, float Age01) { return (6.f + 20.f * FMath::Clamp(Weight, 0.f, 1.f)) * (1.f - FMath::Clamp(Age01, 0.f, 1.f)) + 2.f; }
	static float BurstEmissive(float Weight, float Age01) { const float K = 1.f - FMath::Clamp(Age01, 0.f, 1.f); return (3.f + 12.f * FMath::Clamp(Weight, 0.f, 1.f)) * K * K + 0.5f; }
	static constexpr double BodyClearM = 0.75;
	// True when a mark at P (rink metres) keeps clear of a standing body at C, or of a lying one from Feet along Fall (Fall zero: standing).
	static bool ClearOfBody(const FVector2D& P, const FVector2D& C, const FVector2D& Fall, double Margin);
	// Evolution 10 item 3: the chevron's tip, metres from the checker's centre towards his target (GapM: centre to centre; < 0 = no target: the tip
	// runs TellNearM -> TellFarM along his facing), Since frames after the press, over Windup frames: it leaves his body's edge at the press and reaches
	// the target's near side when the wind-up ends (the window opens), so it meets the man when the contact comes. Scales with whatever the call carries.
	// Evolution 10 (strips: a tip that ran over the wind-up alone met a man already close on the press): the tip covers the way in proportion to the frames
	// left to the contact as they look now (TipRemainFrames: the bodies' gap over the closing speed, never before the wind-up ends), so it stands on him
	// when the bodies meet, at any wind-up length the call carries.
	static double TipRemainFrames(double GapM, double ClosingMps, float Since, int32 Windup);
	static double TipDistM(double GapM, float Since, int32 Windup, double ClosingMps);
	// The tip "meets" the target: within TipMeetM of his near side.
	static constexpr double TipMeetM = 0.25;
	// The shards: how many, and whether a direction (unit, rink plane) may carry one for a victim falling along FallDir (none inside 55 degrees of it).
	static int32 ShardCount(bool bBraced) { return bBraced ? 10 : 6; }
	static bool ShardAllowed(const FVector2D& ShardDir, const FVector2D& FallDir);
	// The countdown: bars still lit for a skater with FramesLeft of Total down frames (12 at the fall, 0 when he gets up).
	static int32 DownBarsLit(int32 FramesLeft, int32 Total);
	// The lean of the drawn figure (degrees, + = top back) Since frames after a check press with Windup frames of wind-up and Window frames of window.
	static float CheckLeanDeg(int32 Since, int32 Windup, int32 Window, int32 EndedAt);

	struct FPress
	{
		int32 Skater = -1;
		int32 Team = 0;
		bool bCross = false;
		bool bHuman = false;
		int64 PressFrame = 0;
		int32 Windup = 0;
		int32 Window = 0;          // body check: the window's frames (12 human, 6 bot)
		int64 EndFrame = -1;       // the frame it landed or missed (-1 running)
		EJamHitResult Result = EJamHitResult::None;
		bool bStripped = false;    // a cross-check's stick took the puck on the press
		int32 Target = -1;         // evolution 10: the man the chevron points at (the nearest opponent ahead at the press; a cross-check's CrossTarget)
		int64 TipMetFrame = -1;    // the first sim frame the tip met him
	};
	struct FBurst
	{
		FVector2D AtM = FVector2D::ZeroVector;
		FVector2D DirM = FVector2D(1, 0);     // checker -> victim
		FVector2D FallM = FVector2D::ZeroVector;   // the victim's fall when the hit downed him (zero: he stays up)
		int64 Frame = 0;
		double RadiusM = 1.0;
		bool bBraced = false;
		FLinearColor Colour = FLinearColor::White;
		float Weight = 0.f;        // evolution 10: BurstWeight
		int32 Checker = -1, Victim = -1;
	};
	struct FDown
	{
		int32 Skater = -1;
		int32 Total = 0;
		int32 Left = 0;
		FVector2D DirM = FVector2D(1, 0);
	};
	struct FFlash
	{
		int64 Frame = 0;
	};
	// One line a hit or a miss (the acceptance run reads JAMHIT).
	struct FHitLog
	{
		int64 Frame = 0;
		FString Kind;              // check | cross | braced
		EJamHitResult Result = EJamHitResult::None;
		int32 Checker = -1, Victim = -1;
		bool bHuman = false;
		double ClosingMps = 0.0;
		double BurstM = 0.0;
		int32 Windup = 0;
		int32 PressToContact = -1;
		int32 TipMet = -1;         // evolution 10: frames from the press to the tip meeting the target (-1 never)
	};

	// After a sim step. SkaterSpeed: game.skater.speed (a check event's Strength is closing / it). TeamColours: the burst and the tells take the checker's.
	void Step(int64 Frame, const TArray<FJamSkaterState>& Skaters, const TArray<FJamSimEvent>& Events, double SkaterSpeed, const FLinearColor TeamColours[2]);
	void Reset();

	// The bars for a render frame. SkaterAtCm / SkaterYawDeg: where each skater is drawn (the figure's interpolated place; index = sim index).
	// PuckAtCm: the drawn puck. The cvar switches are passed in so the tests choose them.
	struct FDrawSwitches { bool bTell = true; bool bBurst = true; bool bDown = true; bool bFlash = true; float Scale = 1.f; bool bStreak = true; bool bBraceBar = true; bool bBraceArc = true;
		// Lane A210 (the quiet marks; the defaults here are package H34's, the game's are jam.hit.Quiet 1): bThinTell a thin white chevron and arc;
		// RingScale < 1 the small contact ring (radius x RingScale, never under RingMinM); bRingTeam false: white, no team colour; RingLife: its share of BurstFrames.
		bool bThinTell = false; float RingScale = 1.f; bool bRingTeam = true; float RingLife = 1.f;
		// bRingByDistance (with RingScale < 1): the ring grows back to its full radius and life between RingNearM and RingFarM from EyeAtCm (the camera).
		bool bRingByDistance = false; FVector EyeAtCm = FVector::ZeroVector; };
	void Draw(double FrameF, const TArray<FVector>& SkaterAtCm, const TArray<float>& SkaterYawDeg, const FVector& PuckAtCm, const FDrawSwitches& Sw, TArray<FJamHitBar>& Out) const;
	// The figure lean for skater I this render frame (degrees, + = top back); 0 = upright.
	float LeanFor(int32 I, double FrameF) const;
	// Lane A210 (jam.hit.Pop): the hit's coil and drive for skater I this render frame, -1 (sunk) .. +1 (sprung); 0 = his own size. CheckPop is the curve (pure).
	static constexpr float PopSettleFrames = 8.f;
	static float CheckPop(float Since, int32 Windup, float End, bool bLanded);
	float PopFor(int32 I, double FrameF) const;

	const TArray<FHitLog>& GetFrameLogs() const { return FrameLogs; }
	const TArray<FPress>& GetPresses() const { return Presses; }
	const TArray<FBurst>& GetBursts() const { return Bursts; }
	const TArray<FDown>& GetDowns() const { return Downs; }
	int32 TeamOf(int32 I) const { return Teams.IsValidIndex(I) ? Teams[I] : 0; }
	FLinearColor ColourOf(int32 Team) const { return Colours[FMath::Clamp(Team, 0, 1)]; }
	// Running counts (the probe line): hits by result, misses, bursts drawn.
	int32 Hits = 0, Misses = 0, Strips = 0;

private:
	TArray<FPress> Presses;     // the live ones and the recently ended (kept while their fade or lean runs)
	TArray<FBurst> Bursts;
	TArray<FDown> Downs;
	TArray<FFlash> Flashes;
	TArray<FHitLog> FrameLogs;
	TArray<bool> Braced;        // per skater: braced this frame
	TArray<double> BracedSpeed;
	TArray<int32> Teams;
	TArray<bool> Down;          // per skater, for the drawing (down skaters show no tells)
	TArray<FVector2D> PosM;     // per skater, the sim's place this frame (the tip's meeting is judged on it)
	TArray<FVector2D> FallM;    // per skater, the fall while down (zero standing)
	TArray<FVector2D> VelM;     // per skater, the sim's velocity this frame (the tip's closing speed)
	TArray<int32> PrevMissed;   // per skater, the sim's ChecksMissed at the last frame (evolution 10: the miss is the sim's own)
	int64 LastFrame = -1;
	FLinearColor Colours[2] = { FLinearColor(1.f, 0.4f, 0.05f), FLinearColor(0.05f, 0.3f, 1.f) };
};

// Evolution 10 item 2: a man down outside the frame. An arrow on the screen's edge (inset EdgeInsetPx) pointing at him, in his team colour: the human's
// mates a hollow diamond with a short arrow, opponents a solid triangle; it stays while he is down and off screen. The radar (UJamRadarWidget) marks
// every downed man (JamHitDowns on the presentation frame). One JAMHIT offscreen line a knockdown it marks (its delay in sim frames).
struct JAMGAME_API FJamEdgeArrow
{
	FVector2D PosPx = FVector2D::ZeroVector;   // the arrow's centre (widget space)
	float AngleRad = 0.f;                      // where it points (screen x right, y down)
	bool bMate = false;
	FLinearColor Colour = FLinearColor::White;
};
namespace JamHitEdge
{
	constexpr float EdgeInsetPx = 56.f;
	constexpr float ArrowPx = 30.f;
	// Off screen: the projected point (widget space; bInFront false when behind the camera) outside the screen. Pure.
	JAMGAME_API bool IsOffScreen(const FVector2D& P, bool bInFront, const FVector2D& Size);
	// The arrow's place on the inset edge along the ray from the screen's centre in Dir (screen space, y down). Pure.
	JAMGAME_API FVector2D EdgePlace(const FVector2D& Size, const FVector2D& Dir, float Inset);
	// The arrow's outline (closed polyline, widget space) by side: a triangle (opponent) or a diamond with a tip (mate).
	JAMGAME_API void Outline(const FJamEdgeArrow& A, TArray<FVector2f>& Out);
}

UCLASS()
class JAMGAME_API UJamHitEdgeWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	TWeakObjectPtr<class AJamHitRead> Source;
	TArray<FJamEdgeArrow> Arrows;
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	TMap<int32, int64> Logged;   // skater -> the down's first frame already logged
};

UCLASS()
class JAMGAME_API AJamHitRead : public AActor
{
	GENERATED_BODY()

public:
	AJamHitRead();

	// The game mode's two calls: after each sim step (once the pawns applied it) and each render frame (after the figures' ApplyRender). They make the
	// actor on first use (Slot holds it). Null world or sim: nothing.
	static void StepSim(UWorld* World, const UJamSim* Sim, const TArray<TObjectPtr<AJamSkater>>& Skaters, TObjectPtr<AJamHitRead>& Slot);
	void Render(const UJamSim* Sim, const TArray<TObjectPtr<AJamSkater>>& Skaters, const FVector& PuckAtCm, double FrameF);

	// 0 off, 1 (default) while the figure lacks device B's take, 2 always.
	static int32 LeanMode();
	static constexpr int32 PrewarmBars = 96;
	// True when the figure declares a function of that name (device B's takes: PlayCheckWindup, PlayCrossCheck, SetCrossCheckHold).
	static bool FigureHas(UObject* Figure, const TCHAR* Function);

	const FJamHitReadModel& GetModel() const { return Model; }
	// Evolution 10: what the edge widget reads (the last render frame's drawn places, the human, the sim frame).
	TArray<FVector> DrawnAtCm;
	int32 HumanTeam = 0;
	int64 SimFrame = 0;
	static bool OffScreenOn();
	int32 GetBarsDrawn() const { return BarsDrawn; }

private:
	void Capture(const UJamSim* Sim);
	void ApplyBars(const TArray<FJamHitBar>& Bars);

	FJamHitReadModel Model;
	UPROPERTY(VisibleAnywhere, Category = "Jam|Hit") TObjectPtr<USceneComponent> Root;
	UPROPERTY(Transient) TArray<TObjectPtr<UStaticMeshComponent>> Pool;
	UPROPERTY(Transient) TArray<TObjectPtr<UMaterialInstanceDynamic>> PoolMids;
	TArray<FLinearColor> PoolColour;
	TArray<float> PoolEmissive;
	int32 BarsDrawn = 0;
	// The capture (jam.hit.Capture): the human's press being filmed, the frame it stops, strips taken a kind.
	int64 CaptureFrom = -1, CaptureUntil = -1;
	FString CaptureKind;
	TMap<FString, int32> CapturesPerKind;
	TArray<bool> LeanApplied;
	UPROPERTY(Transient) TObjectPtr<UJamHitEdgeWidget> Edge;
	// PROPS READY: device B's wind-up takes told per figure (jam.hit.BTakes).
	void TellTakes(const UJamSim* Sim, const TArray<TObjectPtr<AJamSkater>>& Skaters);
	// Lane A210: the braced figure's set (jam.hit.BraceSet): per figure 0..1, 6 sim frames in and out; the drawn figure's scale at K (pure, for the test).
	void StepBraceSet(const UJamSim* Sim);
	void LogStick(const UJamSim* Sim, const TArray<TObjectPtr<AJamSkater>>& Skaters);
	TArray<float> BraceSetK;
	TArray<bool> BraceScaled;
	TArray<float> PopHeld;
	int64 StickPressFrame = -1;
	int32 StickPressWindup = 0;
	void RetimeEarly(const UJamSim* Sim, const TArray<TObjectPtr<AJamSkater>>& Skaters);
public:
	// Lane A210: the frames-to-contact to re-send to the checker's figure when his check lands PressToContact frames after a press with Windup frames of
	// wind-up (1: the take jumps to the frame before its contact pose), or -1 when the take is within a frame of it already. Pure.
	// The drawn figure's scale for a pop of P (-1 sunk: a tenth lower, 6 % wider; +1 sprung: 7 % taller, 4 % narrower). Pure.
	static FVector PopScale(float P) { const float A = FMath::Clamp(P, -2.f, 2.f); return A >= 0.f ? FVector(1.f - 0.04f * A, 1.f - 0.04f * A, 1.f + 0.07f * A) : FVector(1.f - 0.06f * A, 1.f - 0.06f * A, 1.f + 0.10f * A); }
	static constexpr int32 RetimeLeadFrames = 2;
	static int32 RetimeFor(int32 PressToContact, int32 Windup);
	static constexpr float BraceSetFrames = 6.f;
	static FVector BraceSetScale(float K, float Amount) { const float A = FMath::Clamp(K, 0.f, 1.f) * Amount; return FVector(1.f + 0.06f * A, 1.f + 0.06f * A, 1.f - 0.10f * A); }
private:
	TArray<bool> TakesOn, TakesWant;
	TArray<TWeakObjectPtr<UObject>> TakesFig;
};
