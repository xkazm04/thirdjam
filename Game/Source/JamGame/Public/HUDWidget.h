// The match HUD: score as a film counter, "TAKE n", timer, far-net screen marker with distance, radar, Director's Call prompt,
// win/lose cards and the restart hint. A C++ UMG widget whose tree is built in code (no widget blueprint needed). Lane A5.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDFeed.h"
#include "RadarWidget.h"
#include "JamEffectTypes.h"
#include "JamEventBus.h"
#include "HUDWidget.generated.h"

class UCanvasPanel;
class UTextBlock;
class UBorder;
class UImage;
class USceneComponent;

// Where the far-net marker is this frame (Slate units, i.e. DPI-scaled viewport coordinates).
USTRUCT(BlueprintType)
struct JAMGAME_API FJamNetMarker
{
	GENERATED_BODY()

	// Clamped to the viewport with a margin; this is where the diamond is drawn.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Hud")
	FVector2D ScreenPos = FVector2D::ZeroVector;

	// True when the net's own projected point lies inside the viewport and in front of the camera (no clamping needed).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Hud")
	bool bOnScreen = false;

	UPROPERTY(BlueprintReadOnly, Category = "Jam|Hud")
	int32 DistanceM = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Jam|Hud")
	FString Text;
};

UCLASS()
class JAMGAME_API UJamHudWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Creates the HUD for a player and adds it to the viewport.
	UFUNCTION(BlueprintCallable, Category = "Jam|Hud")
	static UJamHudWidget* CreateAndAdd(APlayerController* PC);

	// Pure text helpers (tests call them).
	static FString FormatTimer(int32 FramesRemaining);
	static FString FormatTake(int32 Take, int32 ScoreToWin = 0);
	// Frames of a match during which the controls hint shows. Lane A111 (track U1, B-to-A-0038 fault 6): 8 s (was 10), and only in the first match of a
	// session (process): the hint is spent when its window has run out once or the human has taken a first shot (ControlsHintSpent); later matches show nothing
	// (the pause card has the full table). jam.hud.HintEveryMatch 1 gives back the strip in every match.
	static constexpr int64 ControlsHintFrames = 480;
	static FString ControlsHintText() { return TEXT("SKATE  WASD    BRAKE  PRESS AGAINST YOUR MOTION    AIM  MOUSE    SHOOT  HOLD LMB    PASS  Q    CHECK  RMB    SPRINT  SHIFT"); }
	static bool IsControlsHintSpent() { return bControlsHintSpent; }
	static void SetControlsHintSpent(bool bSpent) { bControlsHintSpent = bSpent; }   // tests and jam.ui.hintreset
	// The live rule: the frame window, and not spent this session (bEveryMatch: the neutral value, the strip in every match).
	static bool ShowControlsHintLive(const FJamPresentationFrame& F, bool bSpent, bool bEveryMatch) { return ShowControlsHint(F) && (bEveryMatch || !bSpent); }
	// ---- Lane A111 (track U1, B-to-A-0038 fault 7: "the human's ring is the only mark among five blue figures"): the marker over the human's helmet, always
	// on while the match plays: a take-yellow chevron with an ink edge (the HUD's accent, not the team's blue, so it reads on clay and on night ice), its tip
	// YouMarkerGapPx over the point YouMarkerHeadM above the ice (the helmet's top is at 1.65 m); the YOU tag rides above it. It stands over the head, so it never covers the stick or the puck.
	// bMarkerOn false (jam.hud.YouMarker 0) is lane A42's rule: the chevron only as the ring fades (20 to 24 m), the tag inside the ring's arc.
	static constexpr float YouMarkerHeadM = 1.8f;
	static constexpr float YouMarkerGapPx = 4.f;
	static constexpr float YouMarkerSizePx = 48.f;   // 34 read as two thin lines at 1280 x 720 (the first still of the proof run)
	static float YouMarkerAlpha(bool bMarkerOn, float RingAlpha) { return bMarkerOn ? 1.f : 1.f - FMath::Clamp(RingAlpha, 0.f, 1.f); }
	// ---- Lane A111 (B-to-A-0038 fault 6): the commentator's strip is docked on the bottom edge (CommentaryDockPx from it), under the play, not 110 px up over
	// the ice; it steps over the controls strip while that shows (first match only) and over the CALL READY line. bDock false is the earlier place.
	static constexpr float CommentaryDockPx = 24.f;
	static float CommentaryStripY(bool bDock, bool bControlsHint, bool bCallLine)
	{
		// The controls strip stands from 140 to 96 px over the edge (lane A8's ready pill is under it): the commentary keeps its place above it for those 8 s.
		if (!bDock || bControlsHint) return bControlsHint ? -150.f : -110.f;
		return bCallLine ? -76.f : -CommentaryDockPx;
	}
	// ---- Lane A111 (B-to-A-0038 fault 6): the distance labels (far net, own net, the off-screen puck) stand on a dark plate (LabelPlateOpacity, LabelPlatePad
	// around the type), and the net badges are not drawn during the goal pause (the sparks and the horn own the net then).
	static constexpr float LabelPlateOpacity = 0.62f;
	static constexpr float LabelPlatePadX = 8.f;
	static constexpr float LabelPlatePadY = 2.f;
	static bool ShowNetBadges(EJamPhase Phase, bool bHideInGoalPause = true) { return !(bHideInGoalPause && Phase == EJamPhase::GoalPause); }
	// Where a plate of a label stands: the label's canvas position moved so the plate overhangs the type by Pad on every side under the same alignment.
	static FVector2D LabelPlatePos(const FVector2D& LabelPos, const FVector2D& Align) { return LabelPos + FVector2D(LabelPlatePadX * (2.f * Align.X - 1.f), LabelPlatePadY * (2.f * Align.Y - 1.f)); }
	// ---- Lane A123 (track U1, evolution 2; docs/experiments/SCREEN.md part 2): nothing sits on the play, and the screen teaches the two new verbs.
	// "The middle third" is the centre rectangle [W/3, 2W/3] x [H/3, 2H/3] of the frame (lane A49's meaning: where the human and the play are).
	static bool InMiddleThird(const FVector2D& Min, const FVector2D& Max, float VW, float VH)
	{
		return Max.X > VW / 3.f && Min.X < VW * 2.f / 3.f && Max.Y > VH / 3.f && Min.Y < VH * 2.f / 3.f;
	}
	// jam.hud.ClearMiddle (1 default, 0 the screen of before): the bark bubbles, the YOU tag and the net labels keep out of the middle third during play.
	static bool ClearMiddleOn();
	// (1) The marker follows the figure: the chevron's tip stands YouMarkerGapPx over the projection of the figure's head anchor (the visual's head_socket, else the
	// top of its bounds, else the point YouMarkerHeadM over the skater's point on the ice: the rule of before, jam.hud.MarkerAnchor 0). HeadLiftCm is added to a
	// mesh-socket anchor along the head's own axis (from the "head" bone to the socket; the world's up where the mesh has no such bone), so it holds for a
	// figure that leans or lies. Device B's meshes carry head_socket about 19 cm over the helmet's top (47 cm over the head bone: the place for a has-puck
	// marker), measured in this lane's first stills (the tip stood 30 px over the helmet at the faceoff): -18 puts the tip on the helmet. The toy's socket
	// (a scene component at 1.65 m) is the helmet's top and takes no lift.
	static constexpr float YouMarkerHeadLiftCm = -18.f;
	static FVector2D YouMarkerTip(const FVector2D& HeadPx) { return HeadPx + FVector2D(0.f, -YouMarkerGapPx); }
	// The chevron's box and the tag's box from the tip (the chevron is YouMarkerSizePx square, its bottom centre on the tip; the tag is about 76 x 34 over it).
	static void YouChevronBox(const FVector2D& Tip, FVector2D& OutMin, FVector2D& OutMax)
	{
		OutMin = Tip + FVector2D(-YouMarkerSizePx * 0.5f, -YouMarkerSizePx); OutMax = Tip + FVector2D(YouMarkerSizePx * 0.5f, 0.f);
	}
	static void YouTagBox(const FVector2D& Tip, FVector2D& OutMin, FVector2D& OutMax)
	{
		OutMin = Tip + FVector2D(-38.f, -(YouMarkerSizePx + 4.f + 34.f)); OutMax = Tip + FVector2D(38.f, -(YouMarkerSizePx + 4.f));
	}
	// The marker never sits on the puck (or the faceoff dot under it): while the puck's projection is inside the chevron's box plus MarkerYieldPadPx the chevron
	// is a ghost (MarkerGhostAlpha); inside the tag's box the tag is not drawn.
	static constexpr float MarkerYieldPadPx = 10.f;
	static constexpr float MarkerGhostAlpha = 0.3f;
	static bool MarkerBoxHasPuck(const FVector2D& Min, const FVector2D& Max, const FVector2D& PuckPx, float Pad = MarkerYieldPadPx)
	{
		return PuckPx.X >= Min.X - Pad && PuckPx.X <= Max.X + Pad && PuckPx.Y >= Min.Y - Pad && PuckPx.Y <= Max.Y + Pad;
	}
	static float MarkerYieldAlpha(bool bPuckInChevronBox) { return bPuckInChevronBox ? MarkerGhostAlpha : 1.f; }
	// (2) The YOU tag is a box over the human: with the middle kept clear it shows while a faceoff counts down (the moment the player looks for himself) and
	// not after the drop; bClearMiddle false is lane A42's window (the first 5 s, 2 s after every drop).
	static bool ShowYouTagLive(bool bClearMiddle, bool bCountingDown, int64 SimFrame, int64 LastDropFrame)
	{
		return bClearMiddle ? bCountingDown : ShowYouTag(SimFrame, LastDropFrame);
	}
	// A net's distance label is not drawn while it would stand in the middle third (the net itself is in front of the player then); the diamond stays.
	static bool ShowNetLabel(bool bClearMiddle, const FVector2D& LabelMin, const FVector2D& LabelMax, float VW, float VH)
	{
		return !(bClearMiddle && InMiddleThird(LabelMin, LabelMax, VW, VH));
	}
	// The scoreboard's centre plate with no timer: "PERIOD n" over "FIRST TO n", centred on the plate (jam.hud.CleanPlate 0: one line on the top border, as before).
	static FString FormatPeriod(int32 Take) { return FString::Printf(TEXT("PERIOD %d"), FMath::Max(1, Take)); }
	static FString FormatGoalRule(int32 ScoreToWin) { return ScoreToWin > 0 ? FString::Printf(TEXT("FIRST TO %d"), ScoreToWin) : FString(); }
	// (3) The two skills of the vision, one short prompt each, once in a session (process), at the first moment it applies, never two at once:
	// Kept: the human's first carry (only where the cartridge keeps the puck: stick.passive_take false). Brake: the first time an opponent closes inside
	// BrakePromptRangeM of the carrying human. A prompt holds PromptFrames sim frames, the last PromptFadeFrames fading. jam.hud.Prompts 0: none.
	enum class EPrompt : uint8 { None, Kept, Brake };
	static constexpr float BrakePromptRangeM = 10.f;
	static constexpr int32 PromptFrames = 300;
	static constexpr int32 PromptFadeFrames = 30;
	static constexpr float PromptSlotY = 112.f;       // under the scoreboard (its plates end near 104): over the stands and the far boards, not the play
	static constexpr float PromptSlotLowY = 206.f;    // under the overlay's line and the effects row while the line shows
	static constexpr float PromptSlotH = 46.f;
	// An edge marker (a net badge, the puck's arrow) at P holds the top slot when it stands in the prompt's box: the prompt then takes the lower slot.
	static bool InPromptSlot(const FVector2D& P, float VW, float HalfW) { return P.Y < PromptSlotY + PromptSlotH + 24.f && FMath::Abs(P.X - VW * 0.5f) < HalfW + 80.f; }
	static float PromptSlotFor(bool bTopSlotTaken) { return bTopSlotTaken ? PromptSlotLowY : PromptSlotY; }
	static FString PromptText(EPrompt P)
	{
		switch (P)
		{
		case EPrompt::Kept: return TEXT("THE PUCK IS YOURS UNTIL SOMEONE PRESSES FOR IT");
		case EPrompt::Brake: return TEXT("S AGAINST YOUR MOTION: STOP, THEN GO");
		default: return FString();
		}
	}
	// What becomes due this frame (the caller shows nothing new while a prompt is up). bPlayLive: phase Play, no faceoff countdown.
	static EPrompt PromptDue(bool bOn, bool bPlayLive, bool bHumanCarries, float NearestOpponentM, bool bClosing, bool bKeptSpent, bool bBrakeSpent, bool bPuckKeptRule)
	{
		if (!bOn || !bPlayLive || !bHumanCarries) return EPrompt::None;
		if (!bKeptSpent && bPuckKeptRule) return EPrompt::Kept;
		if (!bBrakeSpent && bClosing && NearestOpponentM <= BrakePromptRangeM) return EPrompt::Brake;
		return EPrompt::None;
	}
	static float PromptOpacity(int64 ShownFrame, int64 Now)
	{
		const int64 Age = Now - ShownFrame;
		if (ShownFrame < 0 || Age < 0 || Age >= PromptFrames) return 0.f;
		return Age < PromptFrames - PromptFadeFrames ? 1.f : static_cast<float>(PromptFrames - Age) / static_cast<float>(PromptFadeFrames);
	}
	static bool IsPromptSpent(EPrompt P) { return P == EPrompt::Kept ? bKeptPromptSpent : P == EPrompt::Brake ? bBrakePromptSpent : true; }
	static void SetPromptsSpent(bool bKept, bool bBrake) { bKeptPromptSpent = bKept; bBrakePromptSpent = bBrake; }   // tests and jam.ui.promptreset
	// The band of Slate y a prompt holds while one is up (the bark bubbles keep out of it); false when none shows.
	static bool GetPromptBandPx(float& OutTop, float& OutBottom);
	// jam.ui.prompt kept|brake: shows a prompt now (the proof's stills); it counts as spent like a live one.
	void PreviewPrompt(EPrompt P) { PromptPreview = P; }
	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	FString GetPromptText() const;
	// Lane A64: the controls strip stops this far from the right edge: the radar frame (200 + 2 x 8 px) at EdgePx and a 16 px gap.
	static constexpr float ControlsRightInsetPx = 24.f + 216.f + 16.f;
	// Lane A64 (eyes grid premise 2): a notice in the top-left corner for ToastFrames sim frames (4 s), e.g. a rejected -cut cartridge.
	static constexpr int64 ToastFrames = 240;
	// Lane A64: the faceoff countdown / WON ring's x offset from the screen centre (Slate units).
	static constexpr float RingOffsetXPx = 260.f;
	// Lane A123: at 260 the ring's 220 px box reached into the middle third (its DROP / WON / LOST stand half a second into the play); with the middle kept
	// clear (jam.hud.ClearMiddle 1) it stands RingOffsetClearXPx right of centre: its left edge 10 px outside the rectangle at 16:9, the hint line under it.
	static constexpr float RingOffsetClearXPx = 440.f;
	static constexpr float RingBoxPx = 220.f;
	void ShowToast(const FString& Text);
	// Lane A101 (track F1): the contact line, bottom centre above the stamina bar, ContactLineFrames sim frames (the last ContactFadeFrames fade): what just
	// happened to the human's press. The HUD reads the human's counters from the sim (ChecksRefused, ContactRecoils), so a frame the hit-stop held is not lost.
	static constexpr int64 ContactLineFrames = 60;
	static constexpr int64 ContactFadeFrames = 15;
	// Lane A113 (track F1): the soft hit explains itself. The human's pressed check that resolved as a bump (ChecksSoft): TOO SOFT; one that met a skater inside
	// its release grace (ChecksGraced): JUST GOT UP; a window that ran out on nobody (ChecksMissed): MISSED. One line a frame: the grace first (a charge into a
	// skater that just got up can bounce the human too, and the grace is the reason), then the bounce, the refused press, the soft hit, the miss.
	static FString ContactLineFor(bool bRefused, bool bBounced, bool bSoft = false, bool bMissed = false, bool bGraced = false)
	{
		return bGraced ? TEXT("JUST GOT UP") : bBounced ? TEXT("BOUNCED OFF") : bRefused ? TEXT("NOT WITH THE PUCK") : bSoft ? TEXT("TOO SOFT") : bMissed ? TEXT("MISSED") : TEXT("");
	}
	void ShowContactLine(const FString& Text);
	FString GetContactText() const;
	FString GetToastText() const;
	static bool ShowControlsHint(const FJamPresentationFrame& F) { return F.SimFrame < ControlsHintFrames && F.Phase == EJamPhase::Play; }
	// The Directors Call prompt shows only after the first goal.
	static bool ShowDirectorsCall(const FJamPresentationFrame& F) { return F.bDirectorsCallReady && (F.ScoreHome + F.ScoreAway) > 0 && F.Phase != EJamPhase::Win && F.Phase != EJamPhase::Lose; }
	static FString CardFor(EJamPhase Phase);
	// Lane A14 (C-0005): the active-effects strip under the score, "BOLT speed 4.2s    WEIGHT heavy_puck 1.0s" (icon name, effect id, seconds left); empty = hidden.
	static FString FormatEffects(const TArray<FJamEffectHudEntry>& Rows);

	// Applies one frame to every element; called from NativeTick with the latest frame.
	void RefreshFromFrame(const FJamPresentationFrame& Frame);

	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	const FJamNetMarker& GetNetMarker() const { return Marker; }

	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	const FJamNetMarker& GetOwnNetMarker() const { return OwnMarker; }

	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	UJamRadarWidget* GetRadar() const { return Radar; }

	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	FString GetCardText() const;

	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	FString GetScoreText() const;

	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	FString GetTimerText() const;

	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	FString GetTakeText() const;

	// Lane A18 (visual layer). The faceoff countdown reads the feed's FaceoffCountdownFrames when lane A16's field is there; "DROP" shows
	// for DropFrames once it reaches 0. Frames > 0 here previews it (the jam.ui.faceoff console command and the proof shots).
	static constexpr int32 DropFrames = 30;
	void PreviewFaceoff(int32 Frames) { FaceoffPreview = Frames; }
	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	FString GetCountdownText() const;
	// The active-effect slot row: one icon (FJamUiStyle icon name) and a short label per slot, in order; empty hides the row.
	// Lane A14's effect feed calls it (or jam.ui.effects previews it).
	void SetEffectSlots(const TArray<TPair<FString, FString>>& Slots);

	// ---- Lane A42 (HUD pass 2, from the eyes pass of A27): which skater is yours, how a faceoff is won, the markers inside the edge.
	// The "YOU" tag over the human's head: the first 5 s of a match and 2 s after every drop (LastDropFrame -1 = no drop yet).
	static constexpr int64 YouTagStartFrames = 300;
	static constexpr int64 YouTagAfterDropFrames = 120;
	static bool ShowYouTag(int64 SimFrame, int64 LastDropFrame)
	{
		return SimFrame < YouTagStartFrames || (LastDropFrame >= 0 && SimFrame >= LastDropFrame && SimFrame - LastDropFrame < YouTagAfterDropFrames);
	}
	// The ring under the human: 1.1 m in team colour; from RingFadeStartM it fades out and the chevron over the head fades in, gone at RingFadeEndM.
	static constexpr float YouRingRadiusM = 1.1f;
	static constexpr float CarrierRingRadiusM = 0.8f;
	static constexpr float RingFadeStartM = 20.f;
	static constexpr float RingFadeEndM = 24.f;
	static float RingAlphaAt(float CameraDistanceM)
	{
		return 1.f - FMath::Clamp((CameraDistanceM - RingFadeStartM) / (RingFadeEndM - RingFadeStartM), 0.f, 1.f);
	}
	// The word in the ring during a faceoff (R-0013 point 2: the early mover loses, the first clean press wins): HOLD while it counts, DROP at zero.
	static FString FaceoffCue(int32 CountdownFrames, bool bDropped) { return CountdownFrames > 0 ? TEXT("HOLD") : (bDropped ? TEXT("DROP") : TEXT("")); }
	// The one-line hint under the ring for the first FaceoffHintCount faceoffs of a session.
	static constexpr int32 FaceoffHintCount = 3;
	static FString FaceoffHintText() { return TEXT("press shoot at the drop, not before"); }
	// After the draw: WON or LOST in the human's team colour for DrawResultFrames.
	static constexpr int32 DrawResultFrames = 40;
	static FString DrawResultText(bool bWon) { return bWon ? TEXT("WON") : TEXT("LOST"); }
	// Preview of the draw result for the proof shots (jam.ui.draw won|lost [frames]); counts sim frames.
	void PreviewDraw(bool bWon, int32 Frames) { bLastDrawWon = bWon; DrawPreviewFrames = FMath::Max(1, Frames); }
	// Markers stay MarkerEdgePx inside the screen edge (HalfW: half the marker's drawn width, Top: under the scoreboard, Bottom: above the strips).
	static constexpr float MarkerEdgePx = 48.f;
	static FVector2D ClampMarker(FVector2D Sp, float VW, float VH, float HalfW, float Top, float BottomPx)
	{
		return FVector2D(FMath::Clamp(Sp.X, MarkerEdgePx + HalfW, VW - MarkerEdgePx - HalfW), FMath::Clamp(Sp.Y, Top, VH - MarkerEdgePx - BottomPx));
	}
	// The commentator's line (lane A32's JAMVO text) on the bottom-centre strip: two lines at most, held for the line's duration plus
	// CommentaryHoldFrames, then faded over CommentaryFadeFrames. The barks keep their bubbles (one copy of each line on screen).
	static constexpr int32 CommentaryHoldFrames = 90;
	static constexpr int32 CommentaryFadeFrames = 30;
	// Lane A47 (B-to-A-0030 fault 7): the live strip passes HoldFrames 0 and the line's real end (a cut shortens DurationFrames), so the caption
	// fades the moment the voice stops; the default keeps the earlier reading for the tests.
	static float CommentaryOpacityFor(int64 ShownFrame, int32 DurationFrames, int64 Now, int32 HoldFrames = CommentaryHoldFrames)
	{
		const int64 Age = Now - ShownFrame;
		if (Age < 0 || ShownFrame <= -1000000) return 0.f;
		const int64 Hold = static_cast<int64>(FMath::Max(0, DurationFrames)) + FMath::Max(0, HoldFrames);
		if (Age <= Hold) return 1.f;
		const int64 Fade = Age - Hold;
		return Fade >= CommentaryFadeFrames ? 0.f : 1.f - static_cast<float>(Fade) / static_cast<float>(CommentaryFadeFrames);
	}
	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	FString GetCommentaryText() const;
	// What the ring says this frame (HOLD / DROP / WON / LOST / empty); tests and the proof read it.
	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	FString GetFaceoffCue() const;
	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	bool IsYouTagVisible() const;
	// ---- Lane A74 (R-0038): the off-screen puck marker. The camera stays bound to the human; when the puck is outside the inner 85 % of the frame an
	// arrow stands on the frame's edge at the puck's direction with the distance beside it, in the carrier's team colour (grey when loose). It fades in
	// and out over PuckMarkerFadeFrames sim frames; hidden for the faceoff card / countdown, a goal pause, the pause menu, the end cards.
	// ---- Lane A76: the far-net badge stands NetBadgeLiftCm over the crossbar (NetCrossbarCm: the sim default 0.61 m; a taller bar only adds gap) and at least
	// NetBadgeGapPx over the crossbar's projection. EdgeSlotPx is the least distance between two edge elements (the puck keeps its place, the nets slide).
	static constexpr float NetCrossbarCm = 61.f;
	static constexpr float NetBadgeLiftCm = 120.f;
	static constexpr float NetBadgeGapPx = 40.f;
	static constexpr float EdgeSlotPx = 48.f;
	static constexpr float PuckMarkerInnerFrac = 0.85f;
	static constexpr int32 PuckMarkerFadeFrames = 6;
	static constexpr int64 PuckMarkerCardFrames = 150;   // the MATCH card of a fresh match
	// True when Sp (Slate units) is outside the centred inner fraction of the viewport.
	static bool OutsideInnerFrame(const FVector2D& Sp, float VW, float VH, float Frac = PuckMarkerInnerFrac)
	{
		return Sp.X < VW * (1.f - Frac) * 0.5f || Sp.X > VW * (1.f + Frac) * 0.5f || Sp.Y < VH * (1.f - Frac) * 0.5f || Sp.Y > VH * (1.f + Frac) * 0.5f;
	}
	// Where the marker stands: the ray from the screen centre along Dir meets the rectangle given by the four edges.
	static FVector2D PuckMarkerEdgePoint(FVector2D Dir, float VW, float VH, float Left, float Right, float Top, float Bottom)
	{
		if (Dir.IsNearlyZero()) Dir = FVector2D(0.f, 1.f);
		const FVector2D C(VW * 0.5f, VH * 0.5f);
		const float TX = FMath::Abs(Dir.X) > KINDA_SMALL_NUMBER ? (Dir.X > 0 ? Right - C.X : C.X - Left) / FMath::Abs(Dir.X) : TNumericLimits<float>::Max();
		const float TY = FMath::Abs(Dir.Y) > KINDA_SMALL_NUMBER ? (Dir.Y > 0 ? Bottom - C.Y : C.Y - Top) / FMath::Abs(Dir.Y) : TNumericLimits<float>::Max();
		return C + Dir * FMath::Min(TX, TY);
	}
	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	float GetPuckMarkerAlpha() const { return PuckMarkerAlpha; }
	UFUNCTION(BlueprintPure, Category = "Jam|Hud")
	FString GetPuckMarkerText() const { return PuckMarkerText; }
	// Faceoffs seen this session (process), for the hint.
	static int32 GetFaceoffsThisSession() { return FaceoffsThisSession; }
	// The bus event handler: faceoff drop / won / lost (presentation only: nothing is written back).
	UFUNCTION() void HandleJamEvent(FName Event, const FJamEventPayload& Payload);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	// Lane A18 preview counter (declared by lane A16 to keep the tree compiling, R-0003 point 5).
	int32 FaceoffPreview = 0;
	void BuildTree();
	void UpdateMarker(const FJamPresentationFrame& Frame);
	void UpdateOwnMarker(const FJamPresentationFrame& Frame);

	UPROPERTY(Transient) TObjectPtr<UCanvasPanel> Root;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> ScoreHome;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> ScoreAway;
	UPROPERTY(Transient) TObjectPtr<UBorder> HomeBox;
	UPROPERTY(Transient) TObjectPtr<UBorder> AwayBox;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> TakeText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> TimerText;
	UPROPERTY(Transient) TObjectPtr<UBorder> NetDiamond;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> NetText;
	UPROPERTY(Transient) TObjectPtr<UBorder> OwnDiamond;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> OwnText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> ControlsText;
	UPROPERTY(Transient) TObjectPtr<UBorder> ToastPlate;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> ToastText;
	int64 ToastStart = -1;     // sim frame the toast first showed; -1 = waiting for the next frame
	bool bToastPending = false;
	UPROPERTY(Transient) TObjectPtr<UBorder> ContactPlate;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> ContactText;
	int64 ContactStart = -1;   // sim frame the contact line first showed; -1 = none or waiting for the next frame
	bool bContactPending = false;
	int32 ContactSeenRefused = 0, ContactSeenRecoils = 0;
	int32 ContactSeenSoft = 0, ContactSeenMissed = 0, ContactSeenGraced = 0;   // lane A113
	void UpdateContactLine(const FJamPresentationFrame& Frame);
	UPROPERTY(Transient) TObjectPtr<UJamRadarWidget> Radar;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CallText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CardTextBlock;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> HintText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> EffectsText;
	// Lane A18: the plates and strips of the slate kit around the elements above.
	UPROPERTY(Transient) TObjectPtr<UBorder> ControlsStrip;
	UPROPERTY(Transient) TObjectPtr<UBorder> CardPlate;
	// Lane A49: the sprint stamina bar (bottom centre, above the strips): a dark back plate and a fill that shrinks with the stamina; hidden while full and idle.
	UPROPERTY(Transient) TObjectPtr<UBorder> StaminaBack;
	UPROPERTY(Transient) TObjectPtr<UBorder> StaminaFill;
	// Lane A84 (R-0041 point 3): the ability slot right of the stamina bar: a plate with "F  SHOTGUN  READY" and a cooldown fill under it; hidden with no ability.
	UPROPERTY(Transient) TObjectPtr<UBorder> AbilityPlate;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> AbilityText;
	UPROPERTY(Transient) TObjectPtr<UBorder> AbilityFill;
	UPROPERTY(Transient) TObjectPtr<UWidget> CountdownBox;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CountdownText;
	UPROPERTY(Transient) TObjectPtr<class UHorizontalBox> EffectsRow;
	UPROPERTY(Transient) TObjectPtr<UBorder> EffectsPlate;

	FJamNetMarker Marker;
	FJamNetMarker OwnMarker;
	int32 LastFaceoffFrames = 0;
	int64 DropUntil = -1;
	bool bEffectsFromFeed = false;

	// Lane A42: the yours / carrier rings, the YOU tag, the faceoff words, the marker arrows.
	void UpdateYours(const FJamPresentationFrame& Frame);
	// Projects a rink point (metres) with a radius around it to a screen ellipse; false when behind the camera.
	bool ProjectGroundRing(const FVector2D& PosM, float RadiusM, FVector2D& OutCentre, FVector2D& OutSize, float& OutCameraDistM) const;
	void PlaceMarkerArrow(UWidget* Arrow, const FVector2D& Raw, const FVector2D& Clamped, bool bBehind, float VW);
	static int32 FaceoffsThisSession;
	UPROPERTY(Transient) TObjectPtr<UImage> YouRing;
	UPROPERTY(Transient) TObjectPtr<UWidget> YouChevron;
	UPROPERTY(Transient) TObjectPtr<UBorder> YouTag;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> YouTagText;
	UPROPERTY(Transient) TObjectPtr<UImage> CarrierRing;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> FaceoffWord;
	UPROPERTY(Transient) TObjectPtr<UBorder> FaceoffHintPlate;
	UPROPERTY(Transient) TObjectPtr<UWidget> NetArrow;
	UPROPERTY(Transient) TObjectPtr<UWidget> OwnArrow;
	UPROPERTY(Transient) TObjectPtr<UBorder> CommentaryStrip;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CommentaryText;
	UPROPERTY(Transient) TObjectPtr<class UJamEventBus> Bus;
	void UpdateCommentary(const FJamPresentationFrame& Frame);
	void UpdatePuckMarker(const FJamPresentationFrame& Frame, bool bCovered);
	void ResolveEdgeSlots();   // lane A76: one slot rule for the puck chevron, the net chevrons and their labels
	// Lane A111: the dark plates behind the three distance labels follow their labels after the slot rule has run.
	void UpdateLabelPlates();
	UPROPERTY(Transient) TObjectPtr<UBorder> NetPlate;
	UPROPERTY(Transient) TObjectPtr<UBorder> OwnPlate;
	UPROPERTY(Transient) TObjectPtr<UBorder> PuckPlate;
	static bool bControlsHintSpent;
	// ---- Lane A123: the head anchor (found once per visual), the yield, the prompts, the clean centre plate, the marker probe.
	// 2 = the visual's head socket, 1 = the top of its bounds, 0 = none (the caller falls back to the point over the feet).
	int32 HumanHeadWorld(FVector& OutWorld);
	void UpdatePrompts(const FJamPresentationFrame& Frame, bool bCountingDown, bool bSimAdvanced);
	void ProbeMarker(const FJamPresentationFrame& Frame, int32 Source, const FVector2D& Tip, const FVector2D& OldTip, const FVector& HeadWorld);
	TWeakObjectPtr<UObject> HeadVisual;
	TWeakObjectPtr<USceneComponent> HeadComp;
	int32 HeadCompKind = 0;   // 2 a mesh socket, 3 a scene component named head_socket, 1 the bounds of the largest primitive
	float MarkerYield = 1.f;
	float RingXApplied = -1.f;
	bool bCountingThisFrame = false;
	bool bSimAdvancedThisFrame = false;
	static bool bKeptPromptSpent;
	static bool bBrakePromptSpent;
	static uint64 PromptBandTick;           // GFrameCounter of the last tick a prompt was drawn (the band's getter reads it)
	static float PromptBandTop;
	EPrompt PromptNow = EPrompt::None;
	EPrompt PromptPreview = EPrompt::None;
	int64 PromptShown = -1;
	bool bPromptLow = false;   // this prompt took the lower slot (it does not jump back up)
	float LastNearestOpponentM = -1.f;
	TWeakObjectPtr<class UJamDirectorsCallHudWidget> OverlayForPrompt;
	int32 OverlaySearchTick = 0;
	UPROPERTY(Transient) TObjectPtr<UBorder> PromptPlate;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> PromptTextBlock;
	UPROPERTY(Transient) TObjectPtr<UBorder> CentrePlate;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> GoalRuleText;
	int32 CentrePlateMode = -1;
	// The probe's running numbers (jam.hud.MarkerProbe N: a JAMSCREEN marker line every N sim frames).
	struct FMarkerProbe
	{
		int64 Frames = 0, Socket = 0, Bounds = 0, Fallback = 0, Down = 0, LastLogged = -1;
		float TipMaxPx = 0.f, OldMaxPx = 0.f, OldMaxDownPx = 0.f, BoneMaxPx = 0.f, HeadMinCm = 1e9f, HeadMaxCm = 0.f;
	};
	FMarkerProbe Probe;
	bool bHintThisFrame = false;                                  // the controls strip shows this frame (the commentary strip steps over it)
	FLinearColor YouChevronColour = FLinearColor::Transparent;   // the colour the chevron's bars were last built in (re-built on change only)
	UPROPERTY(Transient) TObjectPtr<UWidget> PuckArrow;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> PuckText;
	float PuckMarkerAlpha = 0.f;
	int64 PuckMarkerLastFrame = -1;
	FString PuckMarkerText;
	int64 CommentaryFrame = -1000000;
	int32 CommentaryDuration = 0;
	int64 LastSimFrame = -1;
	int64 LastDropFrame = -1;
	int64 DrawResultUntil = -1;
	int32 DrawPreviewFrames = 0;
	bool bLastDrawWon = false;
	bool bCountdownSeen = false;
};
