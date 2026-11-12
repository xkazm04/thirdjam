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
	// Frames of a match during which the controls hint shows (10 s).
	static constexpr int64 ControlsHintFrames = 600;
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
	UPROPERTY(Transient) TObjectPtr<UJamRadarWidget> Radar;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CallText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CardTextBlock;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> HintText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> EffectsText;
	// Lane A18: the plates and strips of the slate kit around the elements above.
	UPROPERTY(Transient) TObjectPtr<UBorder> ControlsStrip;
	UPROPERTY(Transient) TObjectPtr<UBorder> CardPlate;
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
	int64 CommentaryFrame = -1000000;
	int32 CommentaryDuration = 0;
	int64 LastSimFrame = -1;
	int64 LastDropFrame = -1;
	int64 DrawResultUntil = -1;
	int32 DrawPreviewFrames = 0;
	bool bLastDrawWon = false;
	bool bCountdownSeen = false;
};
